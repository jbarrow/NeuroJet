///////////////////////////////////////////////////////////////////////////////
//
// User.hpp
//
// This file is necessary for NeuroJet compilation.
// It is the only NeuroJet include file that should be modified by the user.
// The comments in this file instruct the user about tailoring NeuroJet for
// personal use.
//
///////////////////////////////////////////////////////////////////////////////

// do not change the next two lines
#if !defined(USER_HPP)
#define USER_HPP

///////////////////////////////////////////////////////////////////////////////
//
// COMPILER
//
// If you are going to compile using g++ or xlC, then make sure BORLAND
// is not defined.
// If you are using any other compiler(Borland, Sun CC, Cray CC, Microsoft),
// then uncomment the next line and make BORLAND defined.
//#define BORLAND
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// USER FUNCTIONS
//
// In this section you should include any files which contain NeuroJet functions
// that you want to be able to use in your scripts.  For example,
// #include "myfunctions.cpp"
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(WEIGHTANALYSIS_HPP)
#   include "WeightAnalysis.hpp"
#endif

///////////////////////////////////////////////////////////////////////////////
//
// BINDING FUNCTIONS AND VARIABLES
//
// In this section you should put your functions into NeuroJet.  This is done by
// editing the following function which NeuroJet will call.  The syntax is
// explained below.
//
// do not change the next line
void BindUserFunctions()
{
// FIXME: This needs to be rewritten
// Variables //////////////////////////////////////////////////////////////////
//
// If you want to create a new external variable that you can access with a
// name in your scripts, then you will need to add three lines.
// First of all, the variable must have been declared a global pointer of
// some type in one of you user include files.  For example, one of your files
// might have had the lines:
//
// int* myintptr;
// float* myfloatptr;
// string mystrptr;
//
// Assuming the variable has been previously defined, then you first must get
// it some new memory, e.g.
//
// myintptr = new int;
// myfloatptr = new float;
// mystrptr = "Hippocampus";
//
// The second thing you must do is give it a value, e.g.
//
// *myintptr = 1;
// *myfloatptr = 3.5f;
//
// The third thing you must do is put it into the NeuroJet program, e.g.
// (The true means read-only, false means you can change the value in your
// script.)
//
// SystemVar::AddIntVar("myint",myintptr,false);
// SystemVar::AddFloatVar("myfloat",myfloatptr,true);
// program::Main().AddStringVar("mystr",mystrptr,false);
//
// @Functions /////////////////////////////////////////////////////////////////
//
// If you have written an @Function {void(ArgListType &arg)} and
// you included it in one of your user include files, then you can put
// it into NeuroJet here.  For example,
//
// SystemVar::AddAtFun("myatfun", myatfun);
//
// ^Functions /////////////////////////////////////////////////////////////////
//
// If you have writtern a ^Function {string (ArgListType &arg)}
// and you included it in one of your user include files, then you can put
// it into NeuroJet here.  For example,
//
// SystemVar::AddCaretFun("mycaretfun", mycaretfun);
//
// A nice way to do this is to make a single function for each of your
// function variable blocks.

   BindWeightAnalysis();

// do not change the next two lines
   return;
}
#endif
// end of user.hpp
///////////////////////////////////////////////////////////////////////////////
