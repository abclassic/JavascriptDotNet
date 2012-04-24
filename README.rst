UPDATE: the JavascriptDotNet repository has been updated recently, after it was found that their repository was out of date(!).
As such, you should not use this repository, and refer to the one mentioned below.

This is a Git clone of the SVN repository of JavascriptDotNet as found on CodePlex at http://javascriptdotnet.codeplex.com/. It includes some fixes that may or may not yet work over there. In particular:

- allows for nesting of Javascript.Run, i.e. Javascript code can call back into .NET which can run more Javascript.
- fixes to the JavascriptObject result which actually makes it work and allows one to extract values and all keys for a given Javascript object result
