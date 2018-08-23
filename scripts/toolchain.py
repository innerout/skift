import os
import shutil
import subprocess
import utils

# --- Utils ------------------------------------------------------------------ #

qemu_flags = ["-m", "256M", "-serial", "mon:stdio", "-M", "accel=kvm:tcg"]

def QEMU(disk):
    #print(" QEMU", disk)
    subprocess.call(["qemu-system-i386", "-cdrom", disk] + qemu_flags)

def MKDIR(directory):
    #print(" MKDIR", directory)

    if not os.path.exists(directory):
        os.makedirs(directory)

    return directory

def RMDIR(directory):
    #print(" RMDIR", directory)

    if os.path.exists(directory):
        shutil.rmtree(directory)

def COPY(src, dest):
    #print(" CP %s -> %s" % (src, dest))
    shutil.copyfile(src, dest)

def TAR(directory, output_file):
    #print(" TAR %s -> %s" % (directory, output_file))
    subprocess.call(["tar", "-cf", output_file, "-C", directory] + os.listdir(directory))

def GRUB(iso, output_file):
    #print(" GRUB %s -> %s" % (iso, output_file))
    command = []

    status = 1

    try:
        status = ubprocess.call(["grub-mkrescue", "-o", output_file, iso])
    except:
        print("grub-mkrescue not found, fallback grub2-mkrescue...")
        status = subprocess.call(["grub2-mkrescue", "-o", output_file, iso])

    return status == 0

# --- Compiler --------------------------------------------------------------- #

def NASM(input_file, output_file):
    if utils.IsUpToDate(output_file, input_file):
        return True

    # print(" AS %s -> %s" % (input_file, output_file))
    command = ["nasm", "-f" "elf32", input_file, "-o", output_file]
    return subprocess.call(command) == 0

def GCC(input_file, output_file, includes, defines, strict):
    if utils.IsUpToDate(output_file, input_file):
        return True

    # print(" CC %s -> %s" % (input_file, output_file))

    defines_flags = []
    for d in defines:
        defines_flags.append("-D%s" % d)

    includes_flags = []
    for i in includes:
        includes_flags.append("-I%s" % i)

    flags = ["gcc", "-m32"]
    flags += ["-fno-pie", "-ffreestanding", "-nostdlib", "-std=gnu11", "-nostdinc"]
    flags += defines_flags
    flags += includes_flags

    if strict:
        flags += ["-Wall", "-Wextra", "-Werror"]

    flags += ["-c", "-o", output_file, input_file ]

    return subprocess.call(flags) == 0

def AR(objects, output_file):
    #print(" AR %i objects -> %s" % (len(objects), output_file))
    command = ["ar", "rcs"] + [output_file] + objects
    return subprocess.call(command) == 0

def LD(objects, libs, output_file, script):
    #print(" LD %i objects (%i libs) using '%s' -> %s" % (len(objects), len(libs), script, output_file))
    command = ["ld"] + ["-melf_i386", "-T", script] + ["-o", output_file] + objects + libs
    return subprocess.call(command) == 0