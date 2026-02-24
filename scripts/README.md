# PPAへ最短で jammy + noble を push する手順

`orig.tar.xz` の不一致を防ぐため、以下を厳守します。

1. `orig` は1回だけ作る
2. `jammy` は `-sa`（orig付き）
3. `noble` は `-sd`（origなし）
4. `noble` の `dch` は `dist/debian/changelog` にだけ入れる

## 前提

- 作業ディレクトリ: `LiVision` ルート
- `scripts/make-dist.sh` の tar 名が対象バージョンになっていること
  - 例: `livision_0.3.3.orig.tar.xz`

## コピペ実行（例: 0.3.3）

```bash
set -euo pipefail
export DEBFULLNAME='Taku Hamazaki'
export DEBEMAIL='hamataku07130713@gmail.com'
VER='0.3.3'
PPA='ppa:hamatakuzaq/livision'

# 1) root changelog を jammy にして orig を1回だけ生成
dch --newversion "${VER}-1~jammy" -D jammy "Release v${VER} for jammy."
bash scripts/make-dist.sh

# 2) jammy: orig付きで source upload
(cd dist && debuild -S -sa)
dput "${PPA}" "livision_${VER}-1~jammy_source.changes"

# 3) noble: dist 側だけ changelog 更新し、origなしで source upload
(
  cd dist
  dch --newversion "${VER}-1~noble" -D noble "Release v${VER} for noble."
  debuild -S -sd
)
dput "${PPA}" "livision_${VER}-1~noble_source.changes"
```

## 失敗時のチェック

- `Rejected: ... orig.tar.xz already exists ... different contents`
  - 原因: `orig` を作り直してしまった
  - 対応: `jammy` で使った `orig` を固定し、`noble` は必ず `-sd` で再作成して再 `dput`
