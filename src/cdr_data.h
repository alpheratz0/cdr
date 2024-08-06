#pragma once

typedef struct {
    int number;
    char *name;
    char *abbr;
} cdr_book;

typedef struct {
    int book;
    int chapter;
    int verse;
    char *text;
} cdr_verse;

extern cdr_verse cdr_verses[];

extern int cdr_verses_length;

extern cdr_book cdr_books[];

extern int cdr_books_length;
