#!/usr/bin/env python3
import os
import subprocess
import sys
def main(encode, size):
    command = "mkdir encoded"
    out = os.popen(command).read()
    command = "ls -p images/ | grep -v / | grep -E 'ppm|pgm'"
    out = os.popen(command).read()
    images = out.split("\n")
    images.pop(-1)
    if(encode): 
        print("encodage en cours... ")
        for image in images:
            command = "./ppm2jpeg images/"+image+" --outfile=encoded/"+image+".jpg --sample="+size
            proc = subprocess.Popen(command, shell=True,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            proc.wait()
            command = "./bin_prof/ppm2blabla images/"+image+" --outfile=encoded/prof_"+image+".jpg --sample="+size
            proc = subprocess.Popen(command, shell=True,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            proc.wait()
            print(image, " encoded")
    k=0
    print("\n debut des comparaisons :")
    while k<len(images):
        command="compare -fuzz 10% -metric AE encoded/prof_"+images[k]+".jpg encoded/"+images[k]+".jpg null"
        proc = subprocess.Popen(command, shell=True,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        proc.wait()
        out = proc.stdout.read()
        if (out==b'0'):
            print(images[k]+" success")
        else:
            print(images[k]+" failed with AE=", out.decode("utf-8"))
        k+=1
encode=True
size="1x1,1x1,1x1"
if (len(sys.argv)>1):
    if sys.argv[1]=="no":
        encode=False
    if(len(sys.argv)>2):
        size=sys.argv[2]
  
main(encode, size)
