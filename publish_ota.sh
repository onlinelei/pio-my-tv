#!/usr/bin/env bash
#
# publish_ota.sh - ESP32 OTA 固件发布脚本
#
# 用法:  ./publish_ota.sh <版本号>
# 示例:  ./publish_ota.sh 1.1.0
#
set -euo pipefail

# ── 配置 ──────────────────────────────────────────────
SERVER_USER="root"
SERVER_HOST="118.145.100.70"
SERVER_OTA_DIR="/root/workSpace/data/nginx/ota"
FIRMWARE_PATH=".pio/build/esp32-s3-n16r8/firmware.bin"
# ──────────────────────────────────────────────────────

# 参数校验
if [[ $# -ne 1 ]]; then
  echo "用法: $0 <版本号>"
  echo "示例: $0 1.1.0"
  exit 1
fi

VERSION="$1"

# 基本版本号格式校验 (x.y.z)
if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "错误: 版本号格式不正确，请使用 x.y.z 格式（如 1.1.0）"
  exit 1
fi

echo "=========================================="
echo " OTA 发布 - 版本 $VERSION"
echo "=========================================="

# Step 1: 编译固件
echo ""
echo "[1/4] 编译固件..."
pio run
if [[ ! -f "$FIRMWARE_PATH" ]]; then
  echo "错误: 编译产物不存在: $FIRMWARE_PATH"
  exit 1
fi
FILE_SIZE=$(stat -f%z "$FIRMWARE_PATH" 2>/dev/null || stat -c%s "$FIRMWARE_PATH")
echo "      编译成功，固件大小: $(( FILE_SIZE / 1024 ))KB"

# Step 2: 计算 MD5
echo ""
echo "[2/4] 计算 MD5..."
MD5=$(md5 -q "$FIRMWARE_PATH" 2>/dev/null || md5sum "$FIRMWARE_PATH" | awk '{print $1}')
echo "      MD5: $MD5"

# Step 3: 归档旧版本固件
echo ""
echo "[3/5] 归档旧版本..."
OLD_VERSION=$(ssh "${SERVER_USER}@${SERVER_HOST}" "cat ${SERVER_OTA_DIR}/manifest.json 2>/dev/null" \
  | grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' | grep -o '"[0-9][^"]*"' | tr -d '"' || echo "")

if [[ -n "$OLD_VERSION" && "$OLD_VERSION" != "$VERSION" ]]; then
  OLD_FIRMWARE="firmware_${OLD_VERSION}.bin"
  ARCHIVE_DIR="${SERVER_OTA_DIR}/firmware_old/${OLD_VERSION}"

  # 检查旧固件文件是否存在
  OLD_EXISTS=$(ssh "${SERVER_USER}@${SERVER_HOST}" "test -f ${SERVER_OTA_DIR}/${OLD_FIRMWARE} && echo yes || echo no")
  if [[ "$OLD_EXISTS" == "yes" ]]; then
    ssh "${SERVER_USER}@${SERVER_HOST}" "mkdir -p ${ARCHIVE_DIR} && \
      mv ${SERVER_OTA_DIR}/${OLD_FIRMWARE} ${ARCHIVE_DIR}/ && \
      cp ${SERVER_OTA_DIR}/manifest.json ${ARCHIVE_DIR}/"
    echo "      旧版本 ${OLD_VERSION} 已归档到: firmware_old/${OLD_VERSION}/"
    echo "      - firmware_${OLD_VERSION}.bin"
    echo "      - manifest.json"
  else
    echo "      未找到旧固件文件 ${OLD_FIRMWARE}，跳过归档"
  fi
elif [[ -n "$OLD_VERSION" && "$OLD_VERSION" == "$VERSION" ]]; then
  echo "      版本号未变化（${VERSION}），跳过归档"
else
  echo "      服务器无旧版本，首次发布，跳过归档"
fi

# Step 4: 上传新固件
FIRMWARE_NAME="firmware_${VERSION}.bin"
echo ""
echo "[4/5] 上传固件到服务器..."
scp "$FIRMWARE_PATH" "${SERVER_USER}@${SERVER_HOST}:${SERVER_OTA_DIR}/${FIRMWARE_NAME}"
echo "      已上传: ${SERVER_OTA_DIR}/${FIRMWARE_NAME}"

# Step 5: 更新 manifest.json
echo ""
echo "[5/5] 更新 manifest.json..."
MANIFEST_CONTENT=$(cat <<EOF
{
  "version": "${VERSION}",
  "url": "${FIRMWARE_NAME}",
  "md5": "${MD5}"
}
EOF
)

ssh "${SERVER_USER}@${SERVER_HOST}" "cat > ${SERVER_OTA_DIR}/manifest.json << 'REMOTE_EOF'
${MANIFEST_CONTENT}
REMOTE_EOF"

echo "      manifest.json 已更新:"
echo "      $(echo "$MANIFEST_CONTENT" | tr '\n' ' ')"

# 验证
echo ""
echo "=========================================="
echo " 验证"
echo "=========================================="
echo "manifest.json 远程内容:"
ssh "${SERVER_USER}@${SERVER_HOST}" "cat ${SERVER_OTA_DIR}/manifest.json"
echo ""
echo ""
echo "发布完成! 设备将在下一个检查周期（默认5分钟）检测到更新。"
