# Design

## Demands

I want to build a very simple system which enable me to take notes on various subject. More precisely, with the following scenario:

- while reading a document, take a side note of a few sentences;
- when a joke comes to mind, write it down and find it later.

I would like to review these information freely, or search by tags.

## Details

Each note should be a file. Files should be written in plain txt that can be searched by filesystem. Additional tags can be searched with regex grammar.

All the notes should be contained in a single folder that can be managed by Git or similar programs. For side note on documents, a *soft* reference should be taken.

A simple cmd program that:

- init a repo;
- create note;
- search by title/tag/content.
