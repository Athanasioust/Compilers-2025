First phase of creating a compiler is about the lexical analysis of the "alpha" language.
We didn't run into much trouble during this phase except the implementation of comments and especially nested comments.
We managed to locate and create a token for the comments using /**/ and for the nested ones but we couldn't display them on the output.


HOW TO RUN:
To run this program you can use the make command of the makefile.
After that you can run the exeutable alpha.out with a test file as an argument.
For example : ./alpha test.txt