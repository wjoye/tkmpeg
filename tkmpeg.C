// Copyright (C) 1999-2004
// Smithsonian Astrophysical Observatory, Cambridge, MA, USA
// For conditions of distribution and use, see copyright notice in "copyright"

#if __GNUC__ >= 3
#include <iostream>
#include <sstream>
using namespace std;
#else
#include <iostream.h>
#include <strstream.h>
#endif

#include <tk.h>

#include "tkmpeg.h"

extern "C" {
  int Tkmpeg_Init(Tcl_Interp* interp);
  int TkmpegCmd(ClientData data, Tcl_Interp *interp, int argc, 
	       const char* argv[]);
}

TkMPEG* tkmpeg=NULL;

int Tkmpeg_Init(Tcl_Interp* interp) {

  // Initialize Frame Widget
  Tcl_CreateCommand(interp, "tkmpeg", TkmpegCmd,
		    (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

  tkmpeg = new TkMPEG(interp);

  // Define Package Name
  if (Tcl_PkgProvide(interp, "saotk", "1.0") == TCL_ERROR)
    return TCL_ERROR;

  return TCL_OK;
}

int TkmpegCmd(ClientData data,Tcl_Interp *interp,int argc,const char* argv[])
{
  if (argc>=2) {
    if (!strncmp(argv[1], "create", 3))
      return tkmpeg->create(argc, argv);
    else if (!strncmp(argv[1], "add", 3))
      return tkmpeg->add(argc, argv);
    else if (!strncmp(argv[1], "close", 3))
      return tkmpeg->finish(argc, argv);
    else {
      Tcl_AppendResult(interp, "tkmpeg: unknown command: ", argv[1], NULL);
      return TCL_ERROR;
    }
  }
  else {
    Tcl_AppendResult(interp, "usage: tkmpeg ?create?close?add?", NULL);
    return TCL_ERROR;
  }
}

TkMPEG::TkMPEG(Tcl_Interp* intp)
{
  width = 512;
  height = 512;
  fps = 25;
  quality = 2;
}

int TkMPEG::create(int argc, const char* argv[])
{
  if (argc == 5) {
    if (argv[0] == '\0') {
	Tcl_AppendResult(interp, "bad filename", NULL);
	return TCL_ERROR;
    }
    {
      string s(argv[1]);
      istringstream str(s);
      str >> width;
    }
    {
      string s(argv[2]);
      istringstream str(s);
      str >> height;
    }
    {
      string s(argv[3]);
      istringstream str(s);
      str >> fps;
    }
    {
      string s(argv[4]);
      istringstream str(s);
      str >> quality;
    }

    if (width < 1) {
      Tcl_AppendResult(interp, "bad width", NULL);
      return TCL_ERROR;
    }
    if (height < 1) {
      Tcl_AppendResult(interp, "bad height", NULL);
      return TCL_ERROR;
    }
    if (fps < 1 || fps > 25) {
      Tcl_AppendResult(interp, "only frame rates between 1 and 25 are suppored", NULL);
      return TCL_ERROR;
    }
    if (quality < 1 || quality > 31) {
      Tcl_AppendResult(interp, "only quality factors between 1 (best) and 31 (worst) are suppored", NULL);
      return TCL_ERROR;
    }

    if(!ezMPEG_Init(&ms, argv[0], width, height, fps, 30, quality)) {
      Tcl_AppendResult(interp, ezMPEG_GetLastError(&ms), NULL);
      return TCL_ERROR;
    }
    if(!ezMPEG_Start(&ms)) {
      Tcl_AppendResult(interp, ezMPEG_GetLastError(&ms), NULL);
      return TCL_ERROR;
    }
  }
  else 
    Tcl_AppendResult(interp, "usage: tkmpeg create ?filename?width?height?fps?quality?", NULL);
    return TCL_ERROR;
}

int TkMPEG::add(int argc, const char* argv[])
{
  unsigned char pict[width*height*3];
  memset(pict,0,width*height*3);

  if(!ezMPEG_Add(&ms, pict)) {
    Tcl_AppendResult(interp, ezMPEG_GetLastError(&ms), NULL);
    return TCL_ERROR;
  }
}

int TkMPEG::finish(int argc, const char* argv[])
{
  if(!ezMPEG_Finalize(&ms)) {
    Tcl_AppendResult(interp, ezMPEG_GetLastError(&ms), NULL);
    return TCL_ERROR;
  }
}

