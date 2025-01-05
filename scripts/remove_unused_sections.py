Import('env')

env.Append(
    LINKFLAGS=[
        "-Wl,--gc-sections",    # 垃圾回收未使用的段
    ]
)