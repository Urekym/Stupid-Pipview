# useless tool

## Why Would You Install This?
honestly, no idea. it's completely useless, and you'll probably regret it. but if you're determined to ruin your day, here's how to do it.

## Installation (Proceed at Your Own Risk)
If you still want to install this masterpiece of nothingness:

i've used [gcc](https://gcc.gnu.org/) (GNU Compiler Collection), and MSYS64 to run it, [this gentle shows you how](https://youtu.be/oC69vlWofJQ) 

```bash
# Clone the repo like there's no tomorrow
git clone https://github.com/Urekym/Stupid-Pipview.git

# Navigate into the abyss
Stupid-Pipview
````
If you want to do some updates on it and run it, make sure to:

inside the files in you cli

```bash
# first 
windres resources.rc -O coff -o resources.res

# second
g++ pip_tool.cpp resources.res -o StupidPipview.exe -mwindows -lcomctl32 -ldwmapi -lgdi32 -luser32
````
## For downloading it in Linux
i don't care, use wine.

