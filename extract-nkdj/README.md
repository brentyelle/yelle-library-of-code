# `extract-nkdj.py`

Provides functionality for searching through the 日本国語大辞典 *Nihon Kokugo Dai-Jiten* (NKDJ), including the ability to save search results to a file.

## Setting Up

1. Ensure that the following Python libraries are installed in your environment:

    * `sys`
    * `pickle`
    * `os`
    * `re`
    * `enum`

2. Clone the git repository [`writemdict`](https://github.com/zhansliu/writemdict) by navigating to your desired directory and running:

```bash
git clone https://github.com/zhansliu/writemdict.git
```

3. Place (or copy) your `.mdx` file of the NKDJ into the same `…/mdict-analysis` directory.
4. Download `extract-nkdj.py` and place it into the same directory.
5. If necessary, change the value of the variable `DICT_FILE` in `extract-nkdj.py` to match your NKDJ `.mdx` file.
6. Run `extract-nkdj.py` by using

```bash
python extract-nkdj.py
```

This first-time run will build the files `__nkdj.pickle`, `__pos.pickle`, and `__romaji.pickle`, which will allow the program to run much more quickly on subsequent runs.

## Searching

To search in the NKDJ, add an instance of the function `search_in_dictionary(…)` to the `main()` function. Multiple searches can be done in sequence! The parameters are:

* `search_list`: Lost of all search parameters (see below for instructions).
* `limit` (default `None`): Maximum number of entries to look for. If this limit is reached, the search is stopped and only the first `limit` results are kept (both for printing to Standard Output and for writing to a file). If  `None`, then the search is unlimited.
* `save_filename` (default `None`): Name (with path) of the file to save the plaintext search results to. If `None`, then the search results are not saved (in which case I recommend `verbose=True`).
* `toss_links` (default `True`): Option to exclude headwords that merely redirect to other entries, i.e., to keep only those headwords that have a definition under them. In my experience, you'll usually want to leave this as the default `True`.
* `strip_html` (default `True`): Option to remove the HTML tags from the definition when writing the search results to a file. (Does nothing if `save_filename=None`).
* `verbose` (default `True`): Option to print every search result to Standard Output.

After adding the `search_in_dictionary`s to the `main()` function, run the program again:

```bash
python extract-nkdj.py
```

### Writing a `search_list`

The format of `search_list` should be as such:

* It is a list of 2-tuples of the form `(search_type, search_terms)`.
* The `search_type` must be a value of type `SearchType`:
    *   `KANA_KEY_CONTAINS` -- the key (w/o kanji) contains any of the given strings
    *   `KANA_KEY_STARTS_WITH` -- the key (w/o kanji) starts with the given string
    *   `KANA_KEY_ENDS_WITH` -- the key (w/o kanji) ends with the given string
    *   `KANA_KEY_EQUALS` -- the key (w/o kanji) is exactly equal to the given string
    *   `ROMAJI_KEY_CONTAINS` -- the key in romaji (w/o kanji) contains any of the given strings
    *   `ROMAJI_KEY_STARTS_WITH` -- the key in romaji (w/o kanji) starts with the given string
    *   `ROMAJI_KEY_ENDS_WITH` -- the key in romaji (w/o kanji) ends with the given string
    *   `ROMAJI_KEY_EQUALS` -- the key in romaji (w/o kanji) is exactly equal to the given string
    *   `FULL_KEY_CONTAINS` -- the key (WITH kanji) contains any of the given strings
    *   `FULL_KEY_LACKS_ALL` -- the key (WITH kanji) lacks ***ALL*** of the given strings
    *   `DEFINITION_CONTAINS` -- the definition contains ANY of the given strings
* Romaji should be in Nihon-Shiki romanization.
* The items of each `search_terms` are on an **or** basis: Only one of them needs to be fulfilled in order for a dictionary entry to match.
    * The exception is `FULL_KEY_LACKS_ALL`, where they are on a **nor** basis: If any of the strings in the `search_terms` are found, then the dictionary entry is omitted from the search results.
* The items of the `search_list` are on an **and** basis: Each of the must be fulfilled in order for a dictionary entry to match.

#### Examples:

I want to find

1. ...words that end in っと or うと or んど or うど,
2. ...and have the kanji 人 or 者 in their full heading,
3. ...and save the results to a file called `hito-onbin.txt`.

```Python
search_in_dictionary(search_list=[
    (SearchType.KANA_KEY_ENDS_WITH, ["うと", "っと", "んど", "うど"]),
    (SearchType.FULL_KEY_CONTAINS,  ["人", "者"])
], save_filename="hito-onbin.txt")
```

I want to find

1. ...words that start with ま,
2. ...that also have っ in them,
3. ...and don't contain the kanji 真,
4. ...and not save the results to a file.

```Python
search_in_dictionary(search_list=[
    (SearchType.KANA_KEY_STARTS_WITH, ["ま"]),
    (SearchType.KANA_KEY_CONTAINS, ["っ"]),
    (SearchType.FULL_KEY_LACKS_ALL,  ["真"])
])
```

I want to find

1. ...words that contain 人 or 者 or 家,
2. ...that also have 非 or 無 or 不 or 否 in them,
3. ...and not print the results to Standard Output,
4. ...but save the results to a file called `negative-people.txt`.

```Python
search_in_dictionary(search_list=[
    (SearchType.FULL_KEY_CONTAINS, ["人", "者", "家"]),
    (SearchType.FULL_KEY_CONTAINS, ["非", "無", "不", "否"]),
], verbose=False, save_filename="negative-people.txt")
```
