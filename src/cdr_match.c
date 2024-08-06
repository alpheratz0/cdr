#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cdr_data.h"
#include "cdr_match.h"
#include "intset.h"

static bool
cdr_verse_matches(const cdr_ref *ref, const cdr_verse *verse)
{
    switch (ref->type) {
        case KJV_REF_SEARCH:
            return (ref->book == 0 || ref->book == verse->book) &&
                (ref->chapter == 0 || verse->chapter == ref->chapter) &&
                regexec(&ref->search, verse->text, 0, NULL, 0) == 0;

        case KJV_REF_EXACT:
            return ref->book == verse->book &&
                (ref->chapter == 0 || ref->chapter == verse->chapter) &&
                (ref->verse == 0 || ref->verse == verse->verse);

        case KJV_REF_EXACT_SET:
            return ref->book == verse->book &&
                (ref->chapter == 0 || verse->chapter == ref->chapter) &&
                intset_contains(ref->verse_set, verse->verse);

        case KJV_REF_RANGE:
            return ref->book == verse->book &&
                ((ref->chapter_end == 0 && ref->chapter == verse->chapter) ||
                    (verse->chapter >= ref->chapter && verse->chapter <= ref->chapter_end)) &&
                (ref->verse == 0 || verse->verse >= ref->verse) &&
                (ref->verse_end == 0 || verse->verse <= ref->verse_end);

        case KJV_REF_RANGE_EXT:
            return ref->book == verse->book &&
                (
                    (verse->chapter == ref->chapter && verse->verse >= ref->verse && ref->chapter != ref->chapter_end) ||
                    (verse->chapter > ref->chapter && verse->chapter < ref->chapter_end) ||
                    (verse->chapter == ref->chapter_end && verse->verse <= ref->verse_end && ref->chapter != ref->chapter_end) ||
                    (ref->chapter == ref->chapter_end && verse->chapter == ref->chapter && verse->verse >= ref->verse && verse->verse <= ref->verse_end)
                );

        default:
            return false;
    }
}

#define KJV_DIRECTION_BEFORE -1
#define KJV_DIRECTION_AFTER 1

static int
cdr_chapter_bounds(int i, int direction, int maximum_steps)
{
    assert(direction == KJV_DIRECTION_BEFORE || direction == KJV_DIRECTION_AFTER);

    int steps = 0;
    for ( ; 0 <= i && i < cdr_verses_length; i += direction) {
        bool step_limit = (maximum_steps != -1 && steps >= maximum_steps) ||
            (direction == KJV_DIRECTION_BEFORE && i == 0) ||
            (direction == KJV_DIRECTION_AFTER && i + 1 == cdr_verses_length);
        if (step_limit) {
            break;
        }

        const cdr_verse *current = &cdr_verses[i], *next = &cdr_verses[i + direction];
        if (current->book != next->book || current->chapter != next->chapter) {
            break;
        }
        steps++;
    }
    return i;
}

static int
cdr_next_match(const cdr_ref *ref, int i)
{
    for ( ; i < cdr_verses_length; i++) {
        const cdr_verse *verse = &cdr_verses[i];
        if (cdr_verse_matches(ref, verse)) {
            return i;
        }
    }
    return -1;
}

static void
cdr_next_addrange(cdr_next_data *next, cdr_range range) {
    if (next->matches[0].start == -1 && next->matches[0].end == -1) {
        next->matches[0] = range;
    } else if (range.start < next->matches[0].end) {
        next->matches[0] = range;
    } else {
        next->matches[1] = range;
    }
}

int
cdr_next_verse(const cdr_ref *ref, const cdr_config *config, cdr_next_data *next)
{
    if (next->current >= cdr_verses_length) {
        return -1;
    }

    if (next->matches[0].start != -1 && next->matches[0].end != -1 && next->current >= next->matches[0].end) {
        next->matches[0] = next->matches[1];
        next->matches[1] = (cdr_range){-1, -1};
    }

    if ((next->next_match == -1 || next->next_match < next->current) && next->next_match < cdr_verses_length) {
        int next_match = cdr_next_match(ref, next->current);
        if (next_match >= 0) {
            next->next_match = next_match;
            cdr_range bounds = {
                .start = cdr_chapter_bounds(next_match, KJV_DIRECTION_BEFORE, config->context_chapter ? -1 : config->context_before),
                .end = cdr_chapter_bounds(next_match, KJV_DIRECTION_AFTER, config->context_chapter ? -1 : config->context_after) + 1,
            };
            cdr_next_addrange(next, bounds);
        } else {
            next_match = cdr_verses_length;
        }
    }

    if (next->matches[0].start == -1 && next->matches[0].end == -1) {
        return -1;
    }

    if (next->current < next->matches[0].start) {
        next->current = next->matches[0].start;
    }

    return next->current++;
}
