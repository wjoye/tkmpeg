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

  // Define Package Name
  if (Tcl_PkgProvide(interp, "tkmpeg", "1.0") == TCL_ERROR)
    return TCL_ERROR;

  // Commands
  Tcl_CreateCommand(interp, "mpeg", TkmpegCmd,
		    (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

  tkmpeg = new TkMPEG(interp);

  if (tkmpeg)
    return TCL_OK;
  else
    return TCL_ERROR;
}

int TkmpegCmd(ClientData data,Tcl_Interp *interp,int argc,const char* argv[])
{
  if (argc>=2) {
    if (!strncmp(argv[1], "create", 3))
      return tkmpeg->create(argc, argv);
    else if (!strncmp(argv[1], "add", 3))
      return tkmpeg->add(argc, argv);
    else if (!strncmp(argv[1], "close", 3))
      return tkmpeg->close(argc, argv);
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
  interp = intp;

  width = 512;
  height = 512;
  quality = 2;
}

int TkMPEG::create(int argc, const char* argv[])
{
  if (argc == 7) {
    if (argv[2] == '\0') {
	Tcl_AppendResult(interp, "bad filename", NULL);
	return TCL_ERROR;
    }
    {
      string s(argv[3]);
      istringstream str(s);
      str >> width;
    }
    {
      string s(argv[4]);
      istringstream str(s);
      str >> height;
    }
    {
      string s(argv[5]);
      istringstream str(s);
      str >> fps;
    }
    {
      string s(argv[6]);
      istringstream str(s);
      str >> quality;
    }

    if(!ezMPEG_Init(&ms, argv[2], width, height, fps, 30, quality)) {
      Tcl_AppendResult(interp, "ezMPEG_Init ", ezMPEG_GetLastError(&ms), NULL);
      return TCL_ERROR;
    }
    if(!ezMPEG_Start(&ms)) {
      Tcl_AppendResult(interp, "ezMPEG_Start ", ezMPEG_GetLastError(&ms),NULL);
      return TCL_ERROR;
    }
  }
  else {
    Tcl_AppendResult(interp, "usage: tkmpeg create <filename> <width> <height> <quality>", NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int TkMPEG::add(int argc, const char* argv[])
{
  if (argv[2] == '\0') {
    Tcl_AppendResult(interp, "bad image name", NULL);
    return TCL_ERROR;
  }
  Tk_PhotoHandle photo = Tk_FindPhoto(interp, argv[2]);
  if (!photo) {
    Tcl_AppendResult(interp, "bad image handle", NULL);
    return TCL_ERROR;
  }
  Tk_PhotoImageBlock block;
  if (!Tk_PhotoGetImage(photo,&block)) {
    Tcl_AppendResult(interp, "bad image block", NULL);
    return TCL_ERROR;
  }

  int w = ms.hsize*16;
  int h = ms.vsize*16;

  unsigned char pict[w*h*3];
  unsigned char* src = block.pixelPtr;
  unsigned char* dst = pict;

  memset(pict,0,w*h*3);
  
  for (int j=0; j<h; j++)
    for (int i=0; i<w; i++) {
      *dst++ = src[(j*width+i)*block.pixelSize+block.offset[0]];
      *dst++ = src[(j*width+i)*block.pixelSize+block.offset[1]];
      *dst++ = src[(j*width+i)*block.pixelSize+block.offset[2]];
    }

  if(!ezMPEG_Add(&ms, pict)) {
    Tcl_AppendResult(interp, "ezMPEG_Add ", ezMPEG_GetLastError(&ms), NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int TkMPEG::close(int argc, const char* argv[])
{
  if(!ezMPEG_Finalize(&ms)) {
    Tcl_AppendResult(interp, "ezMPEG_Finalize", ezMPEG_GetLastError(&ms), NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

