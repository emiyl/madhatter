#include "mh_huffman.h"

#include <stdlib.h>
#include <string.h>

typedef struct mh_huffman_node {
    struct mh_huffman_node *left;
    struct mh_huffman_node *right;
    struct mh_huffman_node *parent;
    uint8_t value;
    int has_value;
    size_t weight;
} mh_huffman_node;

static mh_huffman_node *mh_huffman_node_new(void) {
    mh_huffman_node *node = (mh_huffman_node *)calloc(1u, sizeof(*node));
    if (!node) {
        return NULL;
    }
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->value = 0u;
    node->has_value = 0;
    node->weight = 0u;
    return node;
}

static void mh_huffman_node_free(mh_huffman_node *node) {
    if (!node) {
        return;
    }
    if (node->left) {
        mh_huffman_node_free(node->left);
    }
    if (node->right) {
        mh_huffman_node_free(node->right);
    }
    free(node);
}

static void mh_huffman_get_bool_code(mh_huffman_node *node, uint8_t *out_bits, size_t *out_len) {
    size_t len = 0u;
    mh_huffman_node *current = node;

    while (current && current->parent) {
        mh_huffman_node *parent = current->parent;
        out_bits[len++] = (current == parent->right) ? 1u : 0u;
        current = parent;
    }

    for (size_t i = 0u; i < len / 2u; ++i) {
        uint8_t tmp = out_bits[i];
        out_bits[i] = out_bits[len - 1u - i];
        out_bits[len - 1u - i] = tmp;
    }

    *out_len = len;
}

static int mh_huffman_build_tree(const uint8_t *src, size_t src_len, int half_byte_blocks, mh_huffman_node **root) {
    size_t counts[256] = {0u};
    mh_huffman_node *nodes[256] = {0};
    size_t node_count = 0u;

    if (!src || !root) {
        return -1;
    }

    for (size_t i = 0u; i < src_len; ++i) {
        uint8_t value = src[i];
        if (half_byte_blocks) {
            counts[(value >> 4u) & 0x0Fu] += 1u;
            counts[value & 0x0Fu] += 1u;
        } else {
            counts[value] += 1u;
        }
    }

    for (size_t i = 0u; i < 256u; ++i) {
        if (counts[i] == 0u) {
            continue;
        }

        mh_huffman_node *node = mh_huffman_node_new();
        if (!node) {
            return -1;
        }
        node->value = (uint8_t)i;
        node->has_value = 1;
        node->weight = counts[i];
        nodes[node_count++] = node;
    }

    while (node_count > 1u) {
        for (size_t i = 1u; i < node_count; ++i) {
            mh_huffman_node *key = nodes[i];
            size_t j = i;
            while (j > 0u && nodes[j - 1u]->weight > key->weight) {
                nodes[j] = nodes[j - 1u];
                j -= 1u;
            }
            nodes[j] = key;
        }

        mh_huffman_node *left = nodes[0];
        mh_huffman_node *right = nodes[1];
        mh_huffman_node *parent = mh_huffman_node_new();
        if (!parent) {
            return -1;
        }

        left->parent = parent;
        right->parent = parent;
        parent->left = left;
        parent->right = right;
        parent->weight = left->weight + right->weight;

        for (size_t i = 2u; i < node_count; ++i) {
            nodes[i - 2u] = nodes[i];
        }
        nodes[node_count - 2u] = parent;
        node_count -= 1u;
    }

    *root = (node_count == 0u) ? NULL : nodes[0];
    return 0;
}

static mh_huffman_node *mh_huffman_find_leaf(mh_huffman_node *root, uint8_t value) {
    mh_huffman_node *stack[256] = {0};
    int top = 0;

    if (!root) {
        return NULL;
    }

    stack[top++] = root;
    while (top > 0) {
        mh_huffman_node *node = stack[--top];
        if (!node) {
            continue;
        }
        if (node->has_value && node->value == value) {
            return node;
        }
        if (node->right) {
            stack[top++] = node->right;
        }
        if (node->left) {
            stack[top++] = node->left;
        }
    }

    return NULL;
}

static int mh_huffman_write_code_bits(const uint8_t *bits, size_t bit_count, uint32_t *block, int *bits_remaining, mh_writer *writer) {
    for (size_t i = 0u; i < bit_count; ++i) {
        if (*bits_remaining == 0) {
            if (mh_writer_write_u32_le(writer, *block) != 0) {
                return -1;
            }
            *block = 0u;
            *bits_remaining = 32;
        }
        *bits_remaining -= 1;
        if (bits[i] != 0u) {
            *block |= (1u << *bits_remaining);
        }
    }
    return 0;
}

static int mh_huffman_encode_tree(mh_huffman_node *root, mh_buffer *out) {
    mh_huffman_node **queue = NULL;
    size_t queue_count = 0u;
    size_t cap = 64u;

    if (!root || !out) {
        return -1;
    }

    mh_buffer_init(out);
    queue = (mh_huffman_node **)calloc(cap, sizeof(*queue));
    if (!queue) {
        return -1;
    }

    queue[queue_count++] = root;
    if (mh_buffer_append(out, (const uint8_t[]){0u}, 1u) != 0) {
        free(queue);
        return -1;
    }

    while (queue_count > 0u) {
        mh_huffman_node *node = queue[0];
        memmove(queue, queue + 1, (queue_count - 1u) * sizeof(*queue));
        queue_count -= 1u;

        if (node->has_value) {
            if (mh_buffer_append(out, &node->value, 1u) != 0) {
                free(queue);
                return -1;
            }
            continue;
        }

        if (!node->left || !node->right) {
            free(queue);
            return -1;
        }

        {
            uint8_t temp_data = (uint8_t)((queue_count / 2u) & 0x3Fu);
            if (node->left && node->left->has_value) {
                temp_data |= 0x80u;
            }
            if (node->right && node->right->has_value) {
                temp_data |= 0x40u;
            }
            if (queue_count + 2u >= cap) {
                size_t new_cap = cap * 2u;
                mh_huffman_node **tmp = (mh_huffman_node **)realloc(queue, new_cap * sizeof(*queue));
                if (!tmp) {
                    free(queue);
                    return -1;
                }
                queue = tmp;
                cap = new_cap;
            }
            queue[queue_count++] = node->left;
            queue[queue_count++] = node->right;
            if (mh_buffer_append(out, &temp_data, 1u) != 0) {
                free(queue);
                return -1;
            }
        }
    }

    out->data[0] = (uint8_t)(((out->len / 2u) - 1u) & 0xFFu);
    free(queue);
    return 0;
}

static mh_huffman_node *mh_huffman_decode_tree_node(mh_reader *reader, size_t relative_offset, size_t max_tree_len, int is_data, mh_huffman_node *parent) {
    mh_huffman_node *node = mh_huffman_node_new();
    if (!node) {
        return NULL;
    }
    node->parent = parent;

    if (reader->pos >= max_tree_len) {
        return node;
    }

    if (is_data) {
        node->has_value = 1;
        node->value = mh_reader_read_u8(reader);
        return node;
    }

    {
        const uint8_t temp_byte = mh_reader_read_u8(reader);
        const uint8_t temp_offset = (uint8_t)(temp_byte & 0x3Fu);
        const size_t temp_abs_pos = reader->pos;
        const size_t zero_rel_offset = (relative_offset ^ (relative_offset & 1u)) + ((size_t)temp_offset * 2u) + 2u;
        const size_t child_seek = zero_rel_offset - relative_offset - 1u;
        const int is_left_data = (temp_byte & 0x80u) != 0u;
        const int is_right_data = (temp_byte & 0x40u) != 0u;

        mh_reader_seek(reader, temp_abs_pos + child_seek);
        node->left = mh_huffman_decode_tree_node(reader, zero_rel_offset, max_tree_len, is_left_data, node);
        if (!node->left) {
            mh_huffman_node_free(node);
            return NULL;
        }
        node->right = mh_huffman_decode_tree_node(reader, zero_rel_offset + 1u, max_tree_len, is_right_data, node);
        if (!node->right) {
            mh_huffman_node_free(node);
            return NULL;
        }
        mh_reader_seek(reader, temp_abs_pos);
    }

    return node;
}

int mh_huffman_compress(const uint8_t *src, size_t src_len, mh_buffer *out, int half_byte_blocks) {
    mh_huffman_node *root = NULL;
    mh_writer writer;
    mh_buffer tree = {0};
    uint32_t block = 0u;
    int bits_remaining = 32;

    if (!src || !out) {
        return -1;
    }

    mh_buffer_init(out);
    mh_writer_init(&writer);

    if (mh_huffman_build_tree(src, src_len, half_byte_blocks, &root) != 0) {
        mh_writer_free(&writer);
        return -1;
    }

    if (!root) {
        mh_writer_free(&writer);
        return 0;
    }

    if (mh_writer_write_u8(&writer, (uint8_t)(half_byte_blocks ? 0x24u : 0x28u)) != 0) {
        mh_huffman_node_free(root);
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_u8(&writer, (uint8_t)(src_len & 0xFFu)) != 0) {
        mh_huffman_node_free(root);
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_u8(&writer, (uint8_t)((src_len >> 8u) & 0xFFu)) != 0) {
        mh_huffman_node_free(root);
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_u8(&writer, (uint8_t)((src_len >> 16u) & 0xFFu)) != 0) {
        mh_huffman_node_free(root);
        mh_writer_free(&writer);
        return -1;
    }

    if (mh_huffman_encode_tree(root, &tree) != 0) {
        mh_huffman_node_free(root);
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_bytes(&writer, tree.data, tree.len) != 0) {
        mh_huffman_node_free(root);
        mh_writer_free(&writer);
        mh_buffer_free(&tree);
        return -1;
    }
    mh_buffer_free(&tree);

    for (size_t i = 0u; i < src_len; ++i) {
        uint8_t symbols[2] = {0u, 0u};
        size_t symbol_count = 1u;
        if (half_byte_blocks) {
            symbols[0] = (uint8_t)((src[i] >> 4u) & 0x0Fu);
            symbols[1] = (uint8_t)(src[i] & 0x0Fu);
            symbol_count = 2u;
        } else {
            symbols[0] = src[i];
        }

        for (size_t j = 0u; j < symbol_count; ++j) {
            mh_huffman_node *leaf = mh_huffman_find_leaf(root, symbols[j]);
            uint8_t bits[128] = {0u};
            size_t bit_count = 0u;
            if (!leaf) {
                mh_huffman_node_free(root);
                mh_writer_free(&writer);
                return -1;
            }
            mh_huffman_get_bool_code(leaf, bits, &bit_count);
            if (mh_huffman_write_code_bits(bits, bit_count, &block, &bits_remaining, &writer) != 0) {
                mh_huffman_node_free(root);
                mh_writer_free(&writer);
                return -1;
            }
        }
    }

    if (bits_remaining != 32) {
        if (mh_writer_write_u32_le(&writer, block) != 0) {
            mh_huffman_node_free(root);
            mh_writer_free(&writer);
            return -1;
        }
    }

    {
        size_t align_len = writer.len % 4u;
        if (align_len == 0u) {
            for (size_t i = 0u; i < 4u; ++i) {
                if (mh_writer_write_u8(&writer, 0u) != 0) {
                    mh_huffman_node_free(root);
                    mh_writer_free(&writer);
                    return -1;
                }
            }
        } else {
            for (size_t i = align_len; i < 4u; ++i) {
                if (mh_writer_write_u8(&writer, 0u) != 0) {
                    mh_huffman_node_free(root);
                    mh_writer_free(&writer);
                    return -1;
                }
            }
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    mh_huffman_node_free(root);
    return 0;
}

int mh_huffman_decompress(const uint8_t *src, size_t src_len, mh_buffer *out, int half_byte_blocks) {
    mh_reader reader;
    mh_writer writer;
    uint8_t magic = 0u;
    uint32_t file_size = 0u;
    size_t tree_length = 0u;
    mh_huffman_node *root = NULL;
    mh_huffman_node *current = NULL;
    uint32_t block = 0u;
    size_t bits_left = 0u;
    int is_msb_nibble = 1;
    uint8_t temp_int_data = 0u;
    size_t bytes_written = 0u;

    if (!src || !out || src_len < 5u) {
        return -1;
    }

    mh_buffer_init(out);
    mh_reader_init(&reader, src, src_len);
    mh_writer_init(&writer);

    magic = mh_reader_read_u8(&reader);
    if ((magic & 0xF0u) != 0x20u) {
        return -1;
    }
    half_byte_blocks = ((magic & 0x0Fu) == 0x04u) ? 1 : 0;

    file_size = (uint32_t)mh_reader_read_u8(&reader) |
                ((uint32_t)mh_reader_read_u8(&reader) << 8u) |
                ((uint32_t)mh_reader_read_u8(&reader) << 16u);
    tree_length = (size_t)mh_reader_read_u8(&reader);
    tree_length = (tree_length * 2u) + 1u;

    root = mh_huffman_decode_tree_node(&reader, 5u, 5u + tree_length, 0, NULL);
    if (!root) {
        mh_writer_free(&writer);
        return -1;
    }
    current = root;

    mh_reader_seek(&reader, 5u + tree_length);

    while (writer.len < file_size && reader.pos < reader.len) {
        while (current && !current->has_value) {
            if (bits_left == 0u) {
                if (reader.pos + 4u > reader.len) {
                    mh_huffman_node_free(root);
                    mh_writer_free(&writer);
                    return -1;
                }
                block = mh_reader_read_u32_le(&reader);
                bits_left = 32u;
            }
            bits_left -= 1u;
            current = ((block & (1u << bits_left)) != 0u) ? current->right : current->left;
            if (!current) {
                mh_huffman_node_free(root);
                mh_writer_free(&writer);
                return -1;
            }
        }

        if (!current || !current->has_value) {
            break;
        }

        if (half_byte_blocks) {
            if (is_msb_nibble) {
                temp_int_data = (uint8_t)(current->value << 4u);
                is_msb_nibble = 0;
            } else {
                uint8_t value = (uint8_t)(temp_int_data | current->value);
                if (mh_writer_write_u8(&writer, value) != 0) {
                    mh_huffman_node_free(root);
                    mh_writer_free(&writer);
                    return -1;
                }
                bytes_written += 1u;
                is_msb_nibble = 1;
            }
        } else {
            if (mh_writer_write_u8(&writer, current->value) != 0) {
                mh_huffman_node_free(root);
                mh_writer_free(&writer);
                return -1;
            }
            bytes_written += 1u;
        }

        current = root;
    }

    if (half_byte_blocks && !is_msb_nibble) {
        if (mh_writer_write_u8(&writer, temp_int_data) != 0) {
            mh_huffman_node_free(root);
            mh_writer_free(&writer);
            return -1;
        }
        bytes_written += 1u;
    }

    mh_huffman_node_free(root);
    out->data = writer.data;
    out->len = writer.len;
    return (bytes_written == file_size) ? 0 : -1;
}
