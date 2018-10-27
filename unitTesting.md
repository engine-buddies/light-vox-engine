## Unit Testing Guide in Visual C++

Here are some steps for doing some basic Unit Testing using Visual C++'s
native framework.

You will notice that there are not two `vcprocj` files in the solution, the
`Light Vox Engine` and the `LightVox_UnitTests`

In order to write Unit Tests for your code, there are a few steps simple steps
to take.

1. Add the existing header and source files to the Unit Testing project if you
need to test a new file.
 - Right click on the `Header Files` folder in the Unit Testing project, click
 "Add Existing" and add the necessary `.h` file. This is necessary for proper
 linking of the test project.  
 - Right click on the `Source` folder underneath the `Source Files` folder in
 the Unit Testing project, and add the necessary `.cpp` file.
2. Create a new `TEST_CLASS` if you are testing a new class that doesn't have any existing unit tests.
3. Write you `TEST_METHOD`s inside that test class!  
- There a lot of ways to write your unit tests, but the simplest way is with
the `Assert` functionality. 
