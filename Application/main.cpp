/******************************************************************************/
/*!
\file   main.hpp
\author Wilfred Ng Jun Hwee
\par    DP email: junhweewilfred.ng[at]digipen.edu
\par    course: csd2150
\par    Final Mesh Viewer
\date   04/15/2022
\brief
	This is the implementation of the main entry of the application 
*/
/******************************************************************************/
#include "examples/examples.h"
#include <iostream>
#include <stdexcept>

int main() {

	///////////////////////////////////////////////////////////////////////////
	// DEBUGGER			(2 /2)
	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif
	// END OF DEBUGGER	(2 /2)
	///////////////////////////////////////////////////////////////////////////

	if constexpr (!false) if (auto err = meshViewer(); err) return err;
}
