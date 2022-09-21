#pragma once

//
// Gameplay
//
// Soft camera: Only move the camera when the entity it follows starts getting close to the edge of the screen, and make the camera move towards the entity rather than just snapping to a position
// Clean up the creation of the Player-entity (the IsSpawnPoint-property of tiles)
// Expose all the parameters related to player movement to data and do a tweaking-pass
// Add Saw-hazard
// Create a couple of """real""" levels and make sure that movement-tuning feels atleast okay
// Hook up levels to filewatcher to allow for instant reload whenever a level is resaved in tiled

//
// UI
//
// Create menu-flow, Main menu -> level select -> gameplay -> pause-menu -> level select/main menu
// Make first iteration using IMGUI-widgets
// Create basic UI-solution using sprites/prettier-text-than-ImGUi
	// Does Kenny have a sprite-pack that can be used here?

//
// Engine
//
// Add memory-tracking
	// Generic memorytracker than just tracks all allocation + caller-information to check for leaks
	// Add option to tag allocations and use those tags to group alloations together for display in the profiler
// Add filewatcher or make sure the existing one works
