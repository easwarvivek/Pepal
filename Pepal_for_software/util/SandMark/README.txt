# Do not watermark/modify .jar files in Sandmark/ dir
# Work with java files in watermarkedJARs/ dir
# Watermark Key: 4198716564 or 4198
# creating .jar files
# compile .java file. This will create .class file(s)
javac HelloWorld0.java

# Make a .jar file out of class files
jar cfe {jarfile name}.jar {name of main class} {classfile1}.class {classfile2}.class ...
# e.g
jar cfe HelloWorld0.jar HelloWorld0 HelloWorld0.class

# execute java program
java -jar HelloWorld0.jar


# Static watermark examples
Plain file : watermarkedJARs/HelloWorld0.jar
Zero ("88888") watermark: watermarkedJARs/HelloWorld0_wm0.jar
One ("66666") watermark: watermarkedJARs/HelloWorld0_wm1.jar

# Dynamic Watermarking

javac -cp "../sandmark.jar" TTT.java
jar cfe TTT.jar TTT  TTT.class TTT$1.class TTT$2.class TTT$3.class TTT$4.class
java -cp "../sandmark.jar;TTT.jar" TTT

# When Tracing, add sandmark.jar and TTT.jar in class path 

# Static embedding
java -cp sandmark.jar sandmark.smash.SandmarkCLI -E -A Monden -i .\watermarkedJARs\HelloWorld0.jar -o .\test_wm.jar -k 4198 -w 8888
OR

py -3 .\apdl.py .\watermarkedJARs\HelloWorld0.jar testwm.jar 1
args = rawfile, outputFile, watermarkBit


# static Recognition
java -cp sandmark.jar sandmark.smash.SandmarkCLI -R -A Monden -i .\test_wm.jar -k 4198
OR
py -3 Sandmark\apdl.py testwm.jar 
args = watermarkedFile

========================










SandMark 3.4.0 (Mystique)
-------------------------

SandMark is a tool developed at the University of Arizona 
for examining, obfuscating, watermarking, optimizing, 
measuring, visualizing, decompiling, and attacking Java bytecode. 
The ultimate goal of the project is to implement and study the 
effectiveness of all known software protection algorithms, and
to develop tools to make implementation and evaluation as 
simple as possible.

Currently, SandMark incorporates 
     * 13 static and 3 dynamic software watermarking algorithms
       (Venkatesan, Collberg, Stern,...)
     * 33 code obfuscation algorithms,
     * 3 code optimization algorithms,
     * 6 bytecode diff algorithms (Manber,...),
     * a visualization tool that can display bytecode instructions, 
       a program's inheritance graph, method control flow graphs, 
       register interference graphs, and method slices.
     * 6 software complexity metrics (Harrison, Munson, Kafura,
       Halsted, McCabe, Chidamber).
     * a "wizard" that can select appropriate software watermarking
       and code obfuscation algorithms to protect a program.

SandMark 3.4.0 (Mystique) is our first full release. It is available 
for download from 
   http://sandmark.cs.arizona.edu
SandMark currently runs on Linux, Windows, and MacOS. It requires
Java 1.4.

Software watermarking algorithms can be used to embed
a customer identification number (a fingerprint) into
a Java program in order to trace software pirates. A
SandMark software watermarking algorithm consists of
two programs:
     * The embedder takes a Java jar-file
       and a string (the watermark) as input and produces
       the a new jar-file that embeds the string.
     * The recognizer takes the watermarked
       jar-file as input and produces the watermark
       string as output.
Typically, the watermark is a copyright notice or a
customer identification number.

The code obfuscation algorithms in SandMark take a
Java jar-file as input and produce an obfuscated jar-file
as output. They have many applications:
     * The obfuscations can be used to protect the intellectual
       property of Java programs (by rendering the code difficult
       to understand).
     * Obfuscations can protect fingerprinted programs from collusive
       attacks (by making differently fingerprinted program differ
       everywhere, not just in the parts where the mark is embedded).
     * Obfuscations can also be used to attack software watermarks
       (by reorganizing the code such that the mark can no longer be
       recognized).

SandMark is designed to be simple to use. A graphical user interface
allows novices to easily try out watermarking and obfuscation algorithms.
Algorithms can be combined, the resulting watermarked and/or obfuscated
code can be examined, and attacks can be easily launched.

SandMark is designed to be simple to extend. A plugin-style architecture 
makes is very easy to add new watermarking, obfuscation, and code
optimization algorithms. Additionally, infrastructure is in place to
construct
   * inheritance graphs,
   * interference graphs,
   * control flow graphs, and
   * call graphs;
to perform various static analyzes, such as 
   * stack-simulation,
   * liveness, and
   * defuse;
and a large number of graph algorithms, including graph visualization.

We are currently using SandMark to study which software watermarking
algorithms are vulnerable to which code optimizations and code
obfuscations. We are also interested in evaluating the effectiveness
and performance overhead of obfuscation algorithms.

The development of SandMark is supported by the NSF under grant
CCR-0073483,the AFRL under contract F33615-02-C-1146, and by the
New Economy Research Fund of New Zealand under contracts UOAX9906
and UOAX0214.

------------------------------------------------------------------------
Christian Collberg
Department of Computer Science
The University of Arizona
collberg@cs.arizona.edu

Contact: sandmark-users@listserv.arizona.edu
