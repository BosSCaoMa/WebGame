#!/bin/bash
set -e

# 备份目录和目标目录
SRC_DIR="/var/www/html"
BACKUP_BASE="/var/www/html_backup"
WEB_SRC="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$WEB_SRC/../.." && pwd)"

# 获取当前时间作为备份文件夹名
TIME_STR=$(date +"%Y%m%d_%H%M%S")
BACKUP_DIR="$BACKUP_BASE/$TIME_STR"

# 创建备份目录
mkdir -p "$BACKUP_DIR"

# 备份原有文件
cp -a "$SRC_DIR/." "$BACKUP_DIR/"

# 覆盖复制 web 下所有文件到目标目录
cp -a "$WEB_SRC/." "$SRC_DIR/"

# 同步战斗配置数据，供前端武将配置和装备展示使用
DATA_DIR="$WEB_SRC/../data"
if [[ -f "$DATA_DIR/item_samples.json" ]]; then
	cp -f "$DATA_DIR/item_samples.json" "$SRC_DIR/item_samples.json"
fi
if [[ -f "$DATA_DIR/config/characters.json" ]]; then
	cp -f "$DATA_DIR/config/characters.json" "$SRC_DIR/characters.json"
fi

# 同步展示素材（主视觉 + 武将头像）
IMAGE_DIR="$PROJECT_ROOT/image"
ASSETS_DIR="$SRC_DIR/assets"
HERO_DIR="$ASSETS_DIR/heroes"
mkdir -p "$HERO_DIR"

# 主视觉背景（优先 hero-banner.jpg）
if [[ -f "$IMAGE_DIR/hero-banner.jpg" ]]; then
	cp -f "$IMAGE_DIR/hero-banner.jpg" "$ASSETS_DIR/hero-banner.jpg"
elif [[ -f "$IMAGE_DIR/hero-banner.png" ]]; then
	cp -f "$IMAGE_DIR/hero-banner.png" "$ASSETS_DIR/hero-banner.png"
fi

# 将 image 目录里的其余图片按顺序映射为 2001~2006
mapfile -t HERO_SOURCE_IMAGES < <(
	find "$IMAGE_DIR" -maxdepth 1 -type f \( -iname "*.png" -o -iname "*.jpg" -o -iname "*.jpeg" -o -iname "*.webp" \) \
		! -iname "hero-banner.*" | sort
)

HERO_IDS=(2001 2002 2003 2004 2005 2006)
for idx in "${!HERO_IDS[@]}"; do
	if [[ $idx -lt ${#HERO_SOURCE_IMAGES[@]} ]]; then
		src_img="${HERO_SOURCE_IMAGES[$idx]}"
		dst_img="$HERO_DIR/${HERO_IDS[$idx]}.png"
		cp -f "$src_img" "$dst_img"
	fi
done

echo "备份完成，web文件已更新到 $SRC_DIR。备份目录：$BACKUP_DIR"