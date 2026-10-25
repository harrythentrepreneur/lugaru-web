import sys
args = sys.argv[1:]
if "--modversion" in args:
    print("1.9.5")
    sys.exit(0)
if "--exists" in args:
    sys.exit(0)
if "--cflags" in args:
    print("-I C:/xampp/htdocs/lugaru/source/Dependencies/jsoncpp/include -I C:/xampp/htdocs/lugaru/source/Dependencies")
    sys.exit(0)
if "--libs" in args:
    print("")
    sys.exit(0)
sys.exit(0)
