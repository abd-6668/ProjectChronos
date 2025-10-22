Import("env")

board = env.BoardConfig()

def after_build(source, target, env):
    firmware_path = str(target[0])
    env.Execute(f"elf2bin {firmware_path}")

env.AddPostAction("buildprog", after_build)