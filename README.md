# tiny-regex-c
 Regular expression for MCU. Most rules are supported.
    example : '. ^ $ ? * + {n,m} [...] [^...] \s \S \w \W \d \D |'

# How-To-Use

Just Copy the './src/tiny_regex.c' and './src/tiny_regex.h' to your project.
And using this fucntion of tregex_match_str.
It will return the address of string and length of the string When the first target string is matched.

```c
// matche normal string like strstr();
    tr_res_t test = tregex_match_str("the abc in the string", 0, "abc", 0);
// matche the date string
    tr_res_t test = tregex_match_str("Today is the 2023-12-16", 0, "\d{4}-\d{1,2}-\d{1,2}", 0);

```

# Config the Option

The Function Control
  - TINY_REGEX_CONFIG_DOT_IGNORE_NEWLINE : to control the '.' match the '\r''\n' or not.
  - TINY_REGEX_CONFIG_OR_ENABLE          : to control enable or disable the '|' function.
  - TINY_REGEX_CONFIG_SUBITEM_ENABLE     : to control enable or disable the '()' function.

The Parameter Setting
  - TINY_REGEX_CONFIG_DEPTH_LEVEL        : to setting the nesting depth of '() '.such as '(((...)))'
  - TINY_REGEX_CONFIG_SLICE_NUM          : to maximum slice num by '|' .such as 2 it allow pattern string "item1|item2|tiem3"
  - TINY_REGEX_CONFIG_PATTERN_SIZE       : to setting the pattern string length.0:255 byte,1:65535 byte,other:4,294,967,295 byte

# Version

  - 0.5.0: Test Releasing Version and the '()' is not implemented.