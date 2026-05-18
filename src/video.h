

#ifndef _VIDEO_H
#define _VIDEO_H

/**
 * @file video.h
 * @brief Video output constants and palette storage.
 */

/** Horizontal render resolution in pixels. */
#define	X_RESOLUTION 320
/** Vertical render resolution in pixels. */
#define	Y_RESOLUTION 244

/** Runtime palette used by bitmap rendering (256-color mode). */
static Uint16   palette[256] = { 0 };

#endif
