#!/bin/bash

# 设置错误时立即退出
set -e

# 从version.h文件中读取版本信息
VERSION=$(grep "FIRMWARE_VERSION" src/config/version.h | cut -d'"' -f2)
DESCRIPTION=$(grep "FIRMWARE_DESC" src/config/version.h | cut -d'"' -f2)

# 配置信息
REMOTE_USER="admin"
REMOTE_HOST="8.129.8.11"
REMOTE_DIR="/home/admin/nginx/ota"
BUILD_DIR=".pio/build/nodemcu-32s"
SERVER_URL="http://ota.okeng.top"

# 1. 检查固件文件是否存在
if [ ! -f "${BUILD_DIR}/firmware.bin" ]; then
    echo "Error: firmware.bin not found in ${BUILD_DIR}"
    exit 1
fi

# 2. 计算MD5和文件大小
if [[ "$OSTYPE" == "darwin"* ]]; then
    SIZE=$(stat -f%z "${BUILD_DIR}/firmware.bin")
    MD5=$(md5 -q "${BUILD_DIR}/firmware.bin")
else
    SIZE=$(stat -f%s "${BUILD_DIR}/firmware.bin")
    MD5=$(md5sum "${BUILD_DIR}/firmware.bin" | cut -d' ' -f1)
fi

echo "Firmware size: ${SIZE} bytes"
echo "Firmware MD5: ${MD5}"

# 3. 生成临时version.json
TMP_VERSION_FILE="/tmp/version.json"

cat > ${TMP_VERSION_FILE} << EOF
{
    "version": "${VERSION}",
    "filename": "firmware.bin",
    "description": "${DESCRIPTION}",
    "date": "$(date +%Y-%m-%d)",
    "size": ${SIZE},
    "md5": "${MD5}",
    "url": "${SERVER_URL}/firmware.bin"
}
EOF

# 4. 在远程服务器上创建备份
echo "Creating backup on remote server..."
BACKUP_DIR="${REMOTE_DIR}/firmware_old/v${VERSION}"
ssh ${REMOTE_USER}@${REMOTE_HOST} "cd ${REMOTE_DIR} && \
    if [ -f firmware.bin ]; then \
        mkdir -p ${BACKUP_DIR} && \
        cp firmware.bin ${BACKUP_DIR}/ && \
        cp version.json ${BACKUP_DIR}/; \
    fi"

# 5. 上传新文件到远程服务器
echo "Uploading new firmware..."
scp "${BUILD_DIR}/firmware.bin" ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/
scp ${TMP_VERSION_FILE} ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/version.json

# 6. 清理临时文件
rm ${TMP_VERSION_FILE}

# 7. 验证上传
echo "Verifying uploaded files..."
echo "Version JSON:"
curl -s ${SERVER_URL}/version.json

echo -e "\nFirmware MD5 verification:"
REMOTE_MD5=$(ssh ${REMOTE_USER}@${REMOTE_HOST} "md5sum ${REMOTE_DIR}/firmware.bin" | cut -d' ' -f1)
if [ "$MD5" = "$REMOTE_MD5" ]; then
    echo "✅ MD5 verification successful!"
else
    echo "❌ MD5 verification failed!"
    echo "Local MD5:  ${MD5}"
    echo "Remote MD5: ${REMOTE_MD5}"
    exit 1
fi

echo -e "\n✨ Firmware published successfully to ${SERVER_URL}" 