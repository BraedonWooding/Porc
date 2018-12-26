#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "vector.h"
#include "obsidian.h"

Vector vec_new() {
    Vector vec = malloc(sizeof(struct _vector_t));
    vec->cur_len = vec->max_len = 0;
    vec->data = NULL;
    vec->grow_function = poly_grow_function;
    vec->factor = 2.0;
    return vec;
}

void vec_free(Vector vec) {
    if (vec->data != NULL) free(vec->data);
    free(vec);
}

size_t linear_grow_function(size_t old_len, size_t min_new_len, double factor) {
    obs_assert(factor, >=, 0.0);
    if (factor == 0) return min_new_len;
    return (min_new_len + factor) / factor * factor;
}

size_t poly_grow_function(size_t old_len, size_t min_new_len, double factor) {
    obs_assert(factor, >=, 0.0);
    if (factor == 0) return min_new_len;
    size_t new_len = old_len;
    while (new_len <= min_new_len) new_len *= factor;
    return new_len;
}

size_t exponential_grow_function(size_t old_len, size_t min_new_len, double factor) {
    obs_assert(factor, >=, 0.0);
    if (factor == 0) return min_new_len;
    size_t new_len = old_len;
    while (new_len <= min_new_len) new_len = pow(new_len, factor);
    return new_len;
}

void *vec_at(Vector vec, size_t index) {
    if (vec->cur_len >= index) return NULL;
    return &vec->data[index];
}

void vec_clear_list(Vector vec, bool release_memory) {
    vec->cur_len = 0;
    if (release_memory) {
        vec->max_len = 0;
        free(vec->data);
    }
}

void vec_push_back(Vector vec, void *node) {
    if (vec->cur_len == vec->max_len) vec_reserve(vec, vec->cur_len + 1);
    vec->data[vec->cur_len++] = node;
}

/*
    @Refactor: These two are similar enough that we need a middleman func.
    Also don't like the early return probably an else would be better.
*/
void vec_insert_after(Vector vec, size_t index, void *node) {
    obs_assert(vec->cur_len, >, index);
    if (index == vec->cur_len - 1) {
        vec_push_back(vec, node);
        return;
    }

    if (vec->cur_len == vec->max_len) vec_reserve(vec, vec->cur_len + 1);
    memmove(vec->data + index + 2, vec->data + index + 1, vec->cur_len - 1 - index);
    vec->data[index + 1] = node;
    vec->cur_len++;
}

void vec_insert_before(Vector vec, size_t index, void *node) {
    obs_assert(vec->cur_len, >=, index);
    if (index == vec->cur_len) {
        vec_push_back(vec, node);
        return;
    }

    if (vec->cur_len == vec->max_len) vec_reserve(vec, vec->cur_len + 1);
    memmove(vec->data + index + 1, vec->data + index, vec->cur_len - 1 - index);
    vec->data[index] = node;
    vec->cur_len++;
}

void vec_remove(Vector vec, size_t index) {
    obs_assert(vec->cur_len, >, index);
    // easy remove
    if (index == vec->cur_len - 1) {
        vec->cur_len--;
        return;
    } else {
        // shuffle back one post index
        memmove(vec->data + index, vec->data + index + 1, vec->cur_len - 1 - index);
    }
}

bool vec_is_empty(Vector vec) {
    return vec->cur_len == 0;
}

size_t vec_length(Vector vec) {
    return vec->cur_len;
}

bool vec_will_grow(Vector vec) {
    return vec->cur_len == vec->max_len;
}

size_t vec_capacity(Vector vec) {
    return vec->max_len;
}

void vec_reserve(Vector vec, size_t len) {
    if (vec->max_len >= len) return;
    size_t new_len = vec->grow_function(vec->max_len, len, vec->factor);
    vec->max_len = new_len;
    vec->data = realloc(vec->data, vec->max_len);
}