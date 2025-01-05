Import("env")
from SCons.Script import COMMAND_LINE_TARGETS, DEFAULT_TARGETS

def publish(source, target, env):
    import subprocess
    import os
    from pathlib import Path

    # 检查是否是直接执行 publish 目标
    is_explicit_publish = "publish" in COMMAND_LINE_TARGETS
    
    # 如果不是直接执行 publish 命令，则跳过
    if not is_explicit_publish:
        print("Skipping publish (not explicitly requested)")
        return

    # 获取项目根目录
    project_dir = os.getcwd()
    
    # 发布脚本路径
    publish_script = os.path.join(project_dir, "scripts", "publish_firmware.sh")
    
    print("Publishing firmware...")
    try:
        subprocess.run(['bash', publish_script], check=True)
    except subprocess.CalledProcessError as e:
        print("Error: Failed to publish firmware")
        env.Exit(1)

# 只在明确请求 publish 目标时添加目标
if "publish" in COMMAND_LINE_TARGETS:
    env.AddCustomTarget(
        name="publish",
        dependencies=["buildprog"],  # 保留构建依赖，移除函数中的构建
        actions=[publish],
        title="Publish Firmware",
        description="Build and publish firmware to OTA server"
    ) 