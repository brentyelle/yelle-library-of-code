import readmdict as mdict
import sys
import pickle
import os
import re
from enum import Enum
sys.stdout.reconfigure(encoding='utf-8')

DICT_PATH          = "./"
DICT_FILE          = "nkdj.mdx"
PICKLE_NAME_NKDJ   = "./__nkdj.pickle"
PICKLE_NAME_POS    = "./__pos.pickle"
PICKLE_NAME_ROMAJI = "./__romaji.pickle"
SEPARATOR_STR      = "\t"
SEPARATOR_BIN      = SEPARATOR_STR.encode()
REGEX_DELETE_HTML  = re.compile("<.*?>")
global_dictionary      : dict[bytes, bytes] = None
global_parts_of_speech : dict[str, str]     = None
global_romanization    : dict[str, str]     = None

class SearchType(Enum):
    KANA_KEY_CONTAINS       = 1         # the key (w/o kanji) contains any of the given strings
    KANA_KEY_STARTS_WITH    = 2         # the key (w/o kanji) starts with the given string
    KANA_KEY_ENDS_WITH      = 3         # the key (w/o kanji) ends with the given string
    KANA_KEY_EQUALS         = 4         # the key (w/o kanji) is exactly equal to the given string
    ROMAJI_KEY_CONTAINS     = 5         # the romaji key (w/o kanji) contains any of the given strings
    ROMAJI_KEY_STARTS_WITH  = 6         # the romaji key (w/o kanji) starts with the given string
    ROMAJI_KEY_ENDS_WITH    = 7         # the romaji key (w/o kanji) ends with the given string
    ROMAJI_KEY_EQUALS       = 8         # the romaji key (w/o kanji) is exactly equal to the given string
    FULL_KEY_CONTAINS       = 9         # the key (WITH kanji) contains any of the given strings
    FULL_KEY_LACKS_ALL      = 10        # the key (WITH kanji) lacks ALL of the given strings
    DEFINITION_CONTAINS     = 11        # the definition contains ANY of the given strings

def pickle_setup():
    if not os.path.exists(PICKLE_NAME_POS):
        parts_of_speech = {
            "noun" : "〔名〕",
            "pronoun" : "〔代名〕",
            "adj-ku" : "〔形ク〕",
            "adj-siku": "〔形シク〕",
            "adj-other": "〔形口〕",
            "adj-nari": "〔形動〕",
            "adj-tari": "〔形動タリ〕",
            "adj-naritari": "〔形動ナリ・タリ〕",
            "adverb": "〔副〕",
            "rentaishi": "〔連体〕",
            "conjunction": "〔接続〕",
            "interjection": "〔感動〕",
            "part-case": "〔格助〕",
            "part-adv": "〔副助〕",
            "part-kakari": "〔係助〕",
            "part-final": "〔終助〕",
            "part-interj": "〔間投助〕",
            "verb-aux": "〔助動〕",
            "prefix": "〔接頭〕",
            "suffix": "〔接尾〕",
            "wordroot": "〔語素〕",
            "kanji": "〔字音語素〕",
            "compound": "〔連語〕",
            "makura": "〔枕〕",
        }

        verb_names = []
        verb_types = []
        for cj, ce in zip("カガサタハバマラ", "kgstpbmr"):
            verb_types.append("自" + cj + "四")
            verb_types.append("他" + cj + "四")
            verb_names.append("v4" + ce + "-i")
            verb_names.append("v4" + ce + "-t")
        for cj, ce in zip("カガサタナバマラワ", "kgstnbmrw"):
            verb_types.append("自" + cj + "五")
            verb_types.append("他" + cj + "五")
            verb_names.append("v5" + ce + "-i")
            verb_names.append("v5" + ce + "-t")
        for cj, ce in zip("アカガサザタダナハバマヤラワ", "akgsztdnpbmyrw"):
            verb_types.append("自" + cj + "上一")
            verb_types.append("自" + cj + "上二")
            verb_types.append("自" + cj + "下一")
            verb_types.append("自" + cj + "下二")
            verb_names.append("v1" + ce + "i-i")
            verb_names.append("v2" + ce + "i-i")
            verb_names.append("v1" + ce + "e-i")
            verb_names.append("v2" + ce + "e-i")
            verb_types.append("他" + cj + "上一")
            verb_types.append("他" + cj + "上二")
            verb_types.append("他" + cj + "下一")
            verb_types.append("他" + cj + "下二")
            verb_names.append("v1" + ce + "i-t")
            verb_names.append("v2" + ce + "i-t")
            verb_names.append("v1" + ce + "e-t")
            verb_names.append("v2" + ce + "e-t")
        verb_types.extend(["自カ変", "自サ変", "自ナ変", "自ラ変", "他カ変", "他サ変", "他ナ変", "他ラ変"])
        verb_names.extend(["vik-i", "vis-i", "vin-i", "vir-i", "vik-t", "vis-t", "vin-t", "vir-t"])

        for n, t in zip(verb_names, ["〕" + t + "〕" for t in verb_types]):
            parts_of_speech[n] = t
        with open(PICKLE_NAME_POS, "wb") as f1:
            pickle.dump(parts_of_speech, f1)

    if not os.path.exists(PICKLE_NAME_ROMAJI):
        katakana      = list("アイウエオカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモラリルレロヤユヨワヰヱヲンャュョッァィゥェォヮー")
        hiragana      = list("あいうえおかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもらりるれろやゆよわゐゑをんゃゅょっぁぃぅぇぉゎー")
        romaji_irreg  = ["ya", "yu", "yo", "wa", "wi", "we", "wo", "N", "lya", "lyu", "lyo", "Q", "la", "li", "lu", "le", "lo", "lwa", ":"]
        romanizations = []

        for c in " kgsztdnhbpmr":
            for v in "aiueo":
                romanizations.append(c.rstrip() + v)
        kana2romaji = dict(zip(katakana + hiragana, 2 * (romanizations + romaji_irreg)))

        with open(PICKLE_NAME_ROMAJI, "wb") as f2:
            pickle.dump(kana2romaji, f2)

    if not os.path.exists(PICKLE_NAME_NKDJ):
        print(f"Pickled dictionary `{PICKLE_NAME_NKDJ}` not found. Rebuilding now...")
        with open(PICKLE_NAME_NKDJ, "wb") as f3:
            print("   Loading MDX file...")
            dictionary_generator = mdict.MDX(DICT_PATH + DICT_FILE).items()
            print("   Converting MDX file to Python <dict> object...")
            dictionary_dictform  = dict(dictionary_generator)
            print("   Saving <dict> object with Pickle...")
            pickle.dump(dictionary_dictform, f3)
            print("   Pickling complete!")
    
    return

def romanize_nihon_shiki(kana_string: str):
    REPLACEMENTS  = [("ily", "y" ), ("ile", "ye"),  ("ul", "w"  ), ("ww",  "w" ),
                     ("tel", "t'"), ("del", "d'"),  ("tol", "tw"), ("dol", "dw"),
                     ("Qp",  "pp"), ("Qt",  "tt"),  ("Qk",  "kk"), ("Qs", "ss" ),
                     ("Qb",  "bb"), ("Qd",  "dd"),  ("Qg",  "gg"), ("Qz", "zz" )] 
    romaji_string = ""
    for ch in kana_string:
        romaji_string += (global_romanization[ch] if ch in global_romanization else " ")
    for orig, new in REPLACEMENTS:
        romaji_string = romaji_string.replace(orig, new)
    return romaji_string

def delete_html(original_string: str):
    if '\t' in original_string:
        print("PANIC! TAB CHARACTER FOUND!")
    return re.sub(REGEX_DELETE_HTML, '', original_string)

def is_a_match(key_string           : str,
               key_string_trimmed   : str,
               value_string         : str,
               search_list          : list[tuple[SearchType, str | list[str]]]):
    is_a_match = True
    for search_type, search_terms in search_list:
        search_terms_ensure_list : list[str] = search_terms if isinstance(search_terms, list) else [search_terms]
        romanized_key = romanize_nihon_shiki(key_string_trimmed)
        match search_type:
            case SearchType.KANA_KEY_CONTAINS:
                is_a_match = is_a_match and any(term in key_string_trimmed for term in search_terms_ensure_list)
            case SearchType.KANA_KEY_STARTS_WITH:
                is_a_match = is_a_match and any(key_string_trimmed.startswith(term) for term in search_terms_ensure_list) 
            case SearchType.KANA_KEY_ENDS_WITH:
                is_a_match = is_a_match and any(key_string_trimmed.endswith(term) for term in search_terms_ensure_list) 
            case SearchType.KANA_KEY_EQUALS:
                is_a_match = is_a_match and any(term == key_string_trimmed for term in search_terms_ensure_list)
            case SearchType.FULL_KEY_CONTAINS:
                is_a_match = is_a_match and any(term in key_string for term in search_terms_ensure_list)
            case SearchType.FULL_KEY_LACKS_ALL:
                # note the `all` here!
                is_a_match = is_a_match and all(term not in key_string for term in search_terms_ensure_list)
            case SearchType.DEFINITION_CONTAINS:
                is_a_match = is_a_match and any(term in value_string for term in search_terms_ensure_list)
            case SearchType.ROMAJI_KEY_CONTAINS:
                is_a_match = is_a_match and any(term in romanized_key for term in search_terms_ensure_list)
            case SearchType.ROMAJI_KEY_STARTS_WITH:
                is_a_match = is_a_match and any(romanized_key.startswith(term) for term in search_terms_ensure_list) 
            case SearchType.ROMAJI_KEY_ENDS_WITH:
                is_a_match = is_a_match and any(romanized_key.endswith(term) for term in search_terms_ensure_list) 
            case SearchType.ROMAJI_KEY_EQUALS:
                is_a_match = is_a_match and any(romanized_key == term for term in search_terms_ensure_list) 
            case _:
                raise TypeError
    return is_a_match

def trim_after(input_string : str, char_to_trim_after: str) -> str:
    first_index = input_string.index(char_to_trim_after) if char_to_trim_after in input_string else len(input_string)
    return input_string[:first_index]

def pad_fullwidth(str_to_pad, min_width):
    effective_width = 2 * len(str_to_pad)
    return str_to_pad if effective_width >= min_width else (str_to_pad + " " * (min_width - effective_width))

def search_in_dictionary(search_list    : list[tuple[SearchType, str]],
                         limit          : int  = None,
                         save_filename  : str  = None,
                         toss_links     : bool = True,
                         strip_html     : bool = True,
                         verbose        : bool = False):
    
    print(f"------------------------------------------------------\nAbout to search using the terms:")
    for s_type, s_term in search_list:
        print(f"   [{s_type} {s_term}]")
    input("Press [Enter] to begin.")

    save_file = open(save_filename, "wb") if save_filename else None
    ct        = 0
    for k, v in global_dictionary.items():
        # convert from binary to UTF8 string
        kd: str = k.decode()
        vd: str = v.decode()
        # get rid of kanji listed in brackets
        kd_trim = trim_after(kd, "【").rstrip()
        # toss out any intra-dictionary links, if so desired
        if toss_links and vd.startswith("@@@LINK"):
            continue
        # skip any of the preface files
        if kd.startswith('0'):
            continue
        # check each entry for a match
        if is_a_match(kd, kd_trim, vd, search_list):
            # verbose results
            #print(pad_fullwidth(kd_trim, 30), vd[:50].rstrip()) if verbose else None
            print(kd) if verbose else None
            # manage saving
            if save_file:
                save_file.write(k)
                save_file.write(SEPARATOR_BIN)
                save_file.write(delete_html(vd).encode() if strip_html else v)
            # count the results
            ct += 1
            if (not verbose) and ct % 100 == 0:
                print(f"Found {ct} results so far...")
            if limit and ct >= limit:
                print(f"Reached search {limit=}, so ceasing search.")
                break
    
    print(f"Found {ct} words matching search terms:")
    for s_type, s_term in search_list:
        print(f"   [{s_type} {s_term}]")
    if save_file:
        save_file.close()
        print(f'''Results written to file "{save_filename}".''')
    else:
        print(f"Results not saved.")
    return

## ===============================================================================================================
## ===============================================================================================================
## ===============================================================================================================

def main():
    pickle_setup()

    with open(PICKLE_NAME_POS, "rb") as f1:
        global global_parts_of_speech
        global_parts_of_speech = pickle.load(f1)
        print(f"Loaded part-of-speech object `{PICKLE_NAME_POS}`.")
    with open(PICKLE_NAME_ROMAJI, "rb") as f2:
        global global_romanization
        global_romanization = pickle.load(f2)
        print(f"Loaded romanization object `{PICKLE_NAME_ROMAJI}`.")
    with open(PICKLE_NAME_NKDJ, "rb") as f3:
        global global_dictionary
        global_dictionary = pickle.load(f3)
        print(f"Loaded dictionary object `{PICKLE_NAME_NKDJ}`.")
    
    # ----- ADD SEARCHES BELOW -------


    # ----- ADD SEARCHES ABOVE -------
    return

if __name__ == "__main__":
    main()
