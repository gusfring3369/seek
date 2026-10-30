#!/usr/bin/env sh
# Downloads a 370k-word English list (github.com/dwyl/english-words) for bench/seek_bench.
set -eu
dest="${1:-data/words_alpha.txt}"
curl -fsSL -o "$dest" https://raw.githubusercontent.com/dwyl/english-words/master/words_alpha.txt
echo "Saved $(wc -l < "$dest") words to $dest"
