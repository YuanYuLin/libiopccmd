import ops
import iopc

pkg_path = ""
output_dir = ""
#crosscc = ""
dst_include_dir = ""
PKG_LXC="lxc"

def set_global(args):
    global pkg_path
    global output_dir 
    global crosscc
    global dst_include_dir
    pkg_path = args["pkg_path"]
    output_dir = args["output_path"]
    #crosscc = ops.getEnv("CROSS_COMPILE")
    dst_include_dir = ops.path_join("include",args["pkg_name"])

def MAIN_ENV(args):
    set_global(args)

    if iopc.is_selected_package(PKG_LXC):
        ops.exportEnv(ops.setEnv("SUPPORT_LXC", "y"))

    return False

def MAIN_EXTRACT(args):
    set_global(args)

    ops.copyto(ops.path_join(pkg_path, "src/."), output_dir)

    return True

def MAIN_PATCH(args, patch_group_name):
    set_global(args)
    for patch in iopc.get_patch_list(pkg_path, patch_group_name):
        if iopc.apply_patch(build_dir, patch):
            continue
        else:
            sys.exit(1)

    return True

def MAIN_CONFIGURE(args):
    set_global(args)

    return True

def MAIN_BUILD(args):
    set_global(args)

    #extra_conf = []
    #extra_conf.append("CROSS_COMPILE=" + crosscc)
    #iopc.make(output_dir, extra_conf)
    iopc.make(output_dir)

    return False

def MAIN_INSTALL(args):
    set_global(args)

    iopc.installBin(args["pkg_name"], ops.path_join(output_dir, "include/."), dst_include_dir)
    iopc.installBin(args["pkg_name"], ops.path_join(output_dir, "libiopccmd.so"), "lib")

    return False

def MAIN_SDKENV(args):
    set_global(args)

    cflags = ""
    cflags += " -I" + ops.path_join(iopc.getSdkPath(), 'usr/include/' + args["pkg_name"])
    iopc.add_includes(cflags)

    libs = " -liopccmd"
    iopc.add_libs(libs)

    return False

def MAIN_CLEAN_BUILD(args):
    set_global(args)

    return False

def MAIN(args):
    set_global(args)
    print "linux kernel"

