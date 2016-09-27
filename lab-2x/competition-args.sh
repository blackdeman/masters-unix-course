for v in "$@";do sed '/^https\?:\/\//!s/^/http:\/\//g'<<<"$v";done
