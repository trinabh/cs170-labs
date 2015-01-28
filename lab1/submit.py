#!/usr/bin/python
import sys
import fnmatch
import os
import subprocess

# argv should be labname filename

sub_url = "http://ec2-54-173-71-20.compute-1.amazonaws.com/submit"

def read_auth():
    home_dir = os.getenv("HOME")
    fs = fnmatch.filter(os.listdir(home_dir), "cs202-auth-*")
    if len(fs) == 0:
        print "No auth file found, abort"
        exit()
    with open(os.path.join(home_dir, fs[0])) as f:
        content = f.readlines()
    if len(content) != 2:
        print "Auth file is corrupted, abort"
        exit()
    return (content[0].strip(), content[1].strip())

def print_usage():
    print "Usage: python submit.py <lab-name> <file-name>"

def main(argv):
    if len(argv) != 2:
        print_usage()
        exit()
    l = argv[0]
    f = argv[1]
    (u, p) = read_auth()
    #print u, p
    r = subprocess.Popen(["curl", "--user", u+":"+p, "-X", "POST", "-F", "file=@" + f, "-F", "lab=" + l,  sub_url], stdout=subprocess.PIPE).communicate()[0]
    if r != "ok":
        print "Submission failed, please check parameters / auth file"
    else:
        print '\033[92m' + "Submission successful" + '\033[0m'
    #os.system("curl -X POST -F file=%s -F lab=%s --user %s:%s %s > /dev/null" % (f, l, u, p, sub_url))

if __name__ == "__main__":
    main(sys.argv[1:])
