while read v;do sed '/^https\?:\/\//!s/^/http:\/\//g'<<<"$v";done
