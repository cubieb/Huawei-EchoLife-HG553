/*
  **
  ** conf.c
  **
  ** I originally wrote conf.c as part of the drpoxy package
  ** thanks to Matthew Pratt and others for their additions.
  **
  ** Copyright 1999 Jeroen Vreeken (pe1rxq@chello.nl)
  **
  ** This software is licensed under the terms of the GNU General
  ** Public License (GPL). Please see the file COPYING for details.
  **
  **
*/

/**
 * How to add a config option :
 *
 *   1. think twice, there are settings enough
 *
 *   2. add a field to 'struct config' (conf.h) and 'struct config conf'
 *
 *   4. add a entry to the config_params array below, if your
 *      option should be configurable by the config file.
 */
#include "motion.h"


#if (defined(BSD))
#include "video_freebsd.h"
#else
#include "video.h"
#endif /* BSD */
//add by z67625
#ifndef MDEBUG
#define MDEBUG if(debug) printf("%s: %s %d:",__FILE__, __FUNCTION__, __LINE__); \
              if(debug) printf
#endif
#ifndef MLOG
#define MLOG if(debug) printf
#endif

#define CONFPATH "/var/motion/curcfg.conf"
extern int restart;
extern char *remote_pass;
extern int remote_enable;
extern int debug;
//add by z67625 for lan ip start and end
extern char lanIp[2][16];



#define stripnewline(x) {if ((x)[strlen(x)-1]=='\n') (x)[strlen(x) - 1] = 0; }

//lint -save -e40 -e34 -e10 -e133 -e135 -e64
struct config conf_template = {
	width:                 DEF_WIDTH,
	height:                DEF_HEIGHT,
	quality:               DEF_QUALITY,
	rotate_deg:            0,
	max_changes:           DEF_CHANGES,
	threshold_tune:        0,
	output_normal:         "on",
	motion_img:            0,
	output_all:            0,
	gap:                   DEF_GAP,
	maxmpegtime:           DEF_MAXMPEGTIME,
	snapshot_interval:     0,
	locate:                "off",
	input:                 IN_DEFAULT,
	norm:                  0,
	frame_limit:           DEF_MAXFRAMERATE,
	quiet:                 1,
	ppm:                   0,
	noise:                 DEF_NOISELEVEL,
	noise_tune:            1,
	minimum_frame_time:    0,
	lightswitch:           0,
	nightcomp:             0,
	low_cpu:               0,
	autobright:            0,
	brightness:            0,
	contrast:              0,
	saturation:            0,
	hue:                   0,
	roundrobin_frames:     1,
	roundrobin_skip:       1,
	pre_capture:           0,
	post_capture:          0,
	switchfilter:          0,
	ffmpeg_cap_new:        0,
	ffmpeg_cap_motion:     0,
	ffmpeg_bps:            DEF_FFMPEG_BPS,
	ffmpeg_vbr:            DEF_FFMPEG_VBR,
	ffmpeg_video_codec:    DEF_FFMPEG_CODEC,
	webcam_port:           0,
	webcam_quality:        50,
	webcam_motion:         0,
	webcam_maxrate:        1,
	webcam_localhost:      1,
	webcam_limit:          0,
	control_port:          0,
	control_localhost:     1,
	control_html_output:   1,
	control_authentication:NULL,
	frequency:             0,
	tuner_number:          0,
	timelapse:             0,
	timelapse_mode:        DEF_TIMELAPSE_MODE,
#if (defined(BSD))
	tuner_device:          NULL,
#endif
	video_device:          VIDEO_DEVICE,
	vidpipe:               NULL,
	filepath:              NULL,
	jpegpath:              DEF_JPEGPATH,
	mpegpath:              DEF_MPEGPATH,
	snappath:              DEF_SNAPPATH,
	timepath:              DEF_TIMEPATH,
	on_event_start:        NULL,
	on_event_end:          NULL,
	mask_file:             NULL,
	smart_mask_speed:      0,
	sql_log_image:         1,
	sql_log_snapshot:      1,
	sql_log_mpeg:          0,
	sql_log_timelapse:     0,
	sql_query:             DEF_SQL_QUERY,
	mysql_db:              NULL,
	mysql_host:            "localhost",
	mysql_user:            NULL,
	mysql_password:        NULL,
	on_picture_save:       NULL,
	on_motion_detected:    NULL,
	on_movie_start:        NULL,
	on_movie_end:          NULL,
	motionvidpipe:         NULL,
	netcam_url:            NULL,
	netcam_userpass:       NULL,
	netcam_proxy:          NULL,
	pgsql_db:              NULL,
	pgsql_host:            "localhost",
	pgsql_user:            NULL,
	pgsql_password:        NULL,
	pgsql_port:            5432,
	text_changes:          0,
	text_left:             NULL,
	text_right:            DEF_TIMESTAMP,
	text_event:            DEF_EVENTSTAMP,
	text_double:           0,
	despeckle:             NULL,
	minimum_motion_frames: 1,
	pid_file:              NULL,
};
//lint -restore


static struct context **copy_bool(struct context **, const char *, int);
static struct context **copy_int(struct context **, const char *, int);
static struct context **config_thread(struct context **cnt, const char *str, int val);

static const char *print_bool(struct context **, char **, int, int);
static const char *print_int(struct context **, char **, int, int);
static const char *print_string(struct context **, char **, int, int);
static const char *print_thread(struct context **, char **, int, int);

static void usage(void);

/* Pointer magic to determine relative addresses of variables to a
   struct context pointer */
#define CNT_OFFSET(varname) ( (int)&((struct context *)NULL)->varname )
#define CONF_OFFSET(varname) ( (int)&((struct context *)NULL)->conf.varname ) 
#define TRACK_OFFSET(varname) ( (int)&((struct context *)NULL)->track.varname ) 

config_param config_params[] = {
	{
	"daemon",
	"############################################################\n"
	"# Daemon\n"
	"############################################################\n\n"
	"# Start in daemon (background) mode and release terminal (default: off)",
	CNT_OFFSET(daemon),
	copy_bool,
	print_bool
	},
	{
	"process_id_file",
	"#File to store the process ID, also called pid file. (default: not defined)",
	CONF_OFFSET(pid_file),
	copy_string,
	print_string
	},
	{
	"setup_mode",
	"############################################################\n"
	"# Basic Setup Mode\n"
	"############################################################\n\n"
	"# Start in Setup-Mode, daemon disabled. (default: off)",
	CONF_OFFSET(setup_mode),
	copy_bool,
	print_bool
	},
	{
	"videodevice",
	"\n###########################################################\n"
	"# Capture device options\n"
	"############################################################\n\n"
	"# Videodevice to be used for capturing  (default /dev/video0)\n"
	"# for FreeBSD default is /dev/bktr0",
	CONF_OFFSET(video_device),
	copy_string,
	print_string
	},
#if (defined(BSD))
	{
	"tunerdevice",
	"# Tuner device to be used for capturing using tuner as source (default /dev/tuner0)\n"
	"# This is ONLY used for FreeBSD. Leave it commented out for Linux",
	CONF_OFFSET(tuner_device),
	copy_string,
	print_string
	},
#endif
	{
	"input",
	"# The video input to be used (default: 8)\n"
	"# Should normally be set to 1 for video/TV cards, and 8 for USB cameras",
	CONF_OFFSET(input),
	copy_int,
	print_int
	},
	{
	"norm",
	"# The video norm to use (only for video capture and TV tuner cards)\n"
	"# Values: 0 (PAL), 1 (NTSC), 2 (SECAM), 3 (PAL NC no colour). Default: 0 (PAL)",
	CONF_OFFSET(norm),
	copy_int,
	print_int
	},
	{
	"frequency",
	"# The frequency to set the tuner to (kHz) (only for TV tuner cards) (default: 0)",
	CONF_OFFSET(frequency),
	copy_int,
	print_int
	},
	{
	"rotate",
	"# Rotate image this number of degrees. The rotation affects all saved images as\n"
	"# well as mpeg movies. Valid values: 0 (default = no rotation), 90, 180 and 270.",
	CONF_OFFSET(rotate_deg),
	copy_int,
	print_int
	},
	{
	"width",
	"# Image width (pixels). Valid range: Camera dependent, default: 352",
	CONF_OFFSET(width),
	copy_int,
	print_int
	},
	{
	"height",
	"# Image height (pixels). Valid range: Camera dependent, default: 288",
	CONF_OFFSET(height),
	copy_int,
	print_int
	},
	{
	"framerate",
	"# Maximum number of frames to be captured per second.\n"
	"# Valid range: 2-100. Default: 100 (almost no limit).",
	CONF_OFFSET(frame_limit),
	copy_int,
	print_int
	},
	{
	"minimum_frame_time",
	"# Minimum time in seconds between capturing picture frames from the camera.\n"
	"# Default: 0 = disabled - the capture rate is given by the camera framerate.\n"
	"# This option is used when you want to capture images at a rate lower than 2 per second.",
	CONF_OFFSET(minimum_frame_time),
	copy_int,
	print_int
	},
	{
	"netcam_url",
	"# URL to use if you are using a network camera, size will be autodetected (incl http:// ftp:// or file:///)\n"
	"# Must be a URL that returns single jpeg pictures or a raw mjpeg stream. Default: Not defined",
	CONF_OFFSET(netcam_url),
	copy_string,
	print_string
	},
	{
	"netcam_userpass",
	"# Username and password for network camera (only if required). Default: not defined\n"
	"# Syntax is user:password",
	CONF_OFFSET(netcam_userpass),
	copy_string,
	print_string
	},
	{
	"netcam_proxy",
	"# URL to use for a netcam proxy server, if required, e.g. \"http://myproxy\".\n"
	"# If a port number other than 80 is needed, use \"http://myproxy:1234\".\n"
	"# Default: not defined",
	CONF_OFFSET(netcam_proxy),
	copy_string,
	print_string
	},
	{
	"auto_brightness",
	"# Let motion regulate the brightness of a video device (default: off).\n"
	"# The auto_brightness feature uses the brightness option as its target value.\n"
	"# If brightness is zero auto_brightness will adjust to average brightness value 128.\n"
	"# Only recommended for cameras without auto brightness",
	CONF_OFFSET(autobright),
	copy_bool,
	print_bool
	},
	{
	"brightness",
	"# Set the initial brightness of a video device.\n"
	"# If auto_brightness is enabled, this value defines the average brightness level\n"
	"# which Motion will try and adjust to.\n"
	"# Valid range 0-255, default 0 = disabled",
	CONF_OFFSET(brightness),
	copy_int,
	print_int
	},
	{
	"contrast",
	"# Set the contrast of a video device.\n"
	"# Valid range 0-255, default 0 = disabled",
	CONF_OFFSET(contrast),
	copy_int,
	print_int
	},
	{
	"saturation",
	"# Set the saturation of a video device.\n"
	"# Valid range 0-255, default 0 = disabled",
	CONF_OFFSET(saturation),
	copy_int,
	print_int
	},
	{
	"hue",
	"# Set the hue of a video device (NTSC feature).\n"
	"# Valid range 0-255, default 0 = disabled",
	CONF_OFFSET(hue),
	copy_int,
	print_int
	},

	{
	"roundrobin_frames",
	"\n############################################################\n"
	"# Round Robin (multiple inputs on same video device name)\n"
	"############################################################\n\n"
	"# Number of frames to capture in each roundrobin step (default: 1)",
	CONF_OFFSET(roundrobin_frames),
	copy_int,
	print_int
	},
	{
	"roundrobin_skip",
	"# Number of frames to skip before each roundrobin step (default: 1)",
	CONF_OFFSET(roundrobin_skip),
	copy_int,
	print_int
	},
	{
	"switchfilter",
	"# Try to filter out noise generated by roundrobin (default: off)",
	CONF_OFFSET(switchfilter),
	copy_bool,
	print_bool
	},

	{
	"threshold",
	"\n############################################################\n"
	"# Motion Detection Settings:\n"
	"############################################################\n\n"
	"# Threshold for number of changed pixels in an image that\n"
	"# triggers motion detection (default: 1500)",
	CONF_OFFSET(max_changes),
	copy_int,
	print_int
	},
	{
	"threshold_tune",
	"# Automatically tune the threshold down if possible (default: off)",
	CONF_OFFSET(threshold_tune),
	copy_bool,
	print_bool
	},
	{
	"noise_level",
	"# Noise threshold for the motion detection (default: 32)",
	CONF_OFFSET(noise),
	copy_int,
	print_int
	},
	{
	"noise_tune",
	"# Automatically tune the noise threshold (default: on)",
	CONF_OFFSET(noise_tune),
	copy_bool,
	print_bool
	},
	{
	"night_compensate",
	"# Enables motion to adjust its detection/noise level for very dark frames\n"
	"# Don't use this with noise_tune on. (default: off)",
	CONF_OFFSET(nightcomp),
	copy_bool,
	print_bool
	},
	{
	"despeckle",
	"# Despeckle motion image using (e)rode or (d)ilate or (l)abel (Default: not defined)\n"
	"# Recommended value is EedDl. Any combination (and number of) of E, e, d, and D is valid.\n"
	"# (l)abeling must only be used once and the 'l' must be the last letter.\n"
	"# Comment out to disable",
	CONF_OFFSET(despeckle),
	copy_string,
	print_string
	},
	{
	"mask_file",
	"# PGM file to use as a sensitivity mask.\n"
	"# Full path name to. (Default: not defined)",
	CONF_OFFSET(mask_file),
	copy_string,
	print_string
	},
	{
	"smart_mask_speed",
	"# Dynamically create a mask file during operation (default: 0)\n"
	"# Adjust speed of mask changes from 0 (off) to 10 (fast)",
	CONF_OFFSET(smart_mask_speed),
	copy_int,
	print_int
	},
	{
	"lightswitch",
	"# Ignore sudden massive light intensity changes given as a percentage of the picture\n"
	"# area that changed intensity. Valid range: 0 - 100 , default: 0 = disabled",
	CONF_OFFSET(lightswitch),
	copy_int,
	print_int
	},
	{
	"minimum_motion_frames",
	"# Picture frames must contain motion at least the specified number of frames\n"
	"# in a row before they are detected as true motion. At the default of 1, all\n"
	"# motion is detected. Valid range: 1 to thousands, recommended 1-5",
	CONF_OFFSET(minimum_motion_frames),
	copy_int,
	print_int
	},
	{
	"pre_capture",
	"# Specifies the number of pre-captured (buffered) pictures from before motion\n"
	"# was detected that will be output at motion detection.\n"
	"# Recommended range: 0 to 5 (default: 0)\n"
	"# Do not use large values! Large values will cause Motion to skip video frames and\n"
	"# cause unsmooth mpegs. To smooth mpegs use larger values of post_capture instead.",
	CONF_OFFSET(pre_capture),
	copy_int,
	print_int
	},
	{
	"post_capture",
	"# Number of frames to capture after motion is no longer detected (default: 0)",
	CONF_OFFSET(post_capture),
	copy_int,
	print_int
	},
	{
	"gap",
	"# Gap is the seconds of no motion detection that triggers the end of an event\n"
	"# An event is defined as a series of motion images taken within a short timeframe.\n"
	"# Recommended value is 60 seconds (Default). The value 0 is allowed and disables\n"
	"# events causing all Motion to be written to one single mpeg file and no pre_capture.",
	CONF_OFFSET(gap),
	copy_int,
	print_int
	},
	{
	"max_mpeg_time",
	"# Maximum length in seconds of an mpeg movie\n"
	"# When value is exceeded a new mpeg file is created. (Default: 0 = infinite)",
	CONF_OFFSET(maxmpegtime),
	copy_int,
	print_int
	},
	{
	"low_cpu",
	"# Number of frames per second to capture when not detecting\n"
	"# motion (saves CPU load) (Default: 0 = disabled)",
	CONF_OFFSET(low_cpu),
	copy_int,
	print_int
	},
	{
	"output_all",
	"# Always save images even if there was no motion (default: off)",
	CONF_OFFSET(output_all),
	copy_bool,
	print_bool
	},

	{
	"output_normal",
	"\n############################################################\n"
	"# Image File Output\n"
	"############################################################\n\n"
	"# Output 'normal' pictures when motion is detected (default: on)\n"
	"# Valid values: on, off, first, best\n"
	"# When set to 'first', only the first picture of an event is saved.\n"
	"# Picture with most motion of an event is saved when set to 'best'.\n"
	"# Can be used as preview shot for the corresponding movie.",
	CONF_OFFSET(output_normal),
	copy_string,
	print_string
	},
	{
	"output_motion",
	"# Output pictures with only the pixels moving object (ghost images) (default: off)",
	CONF_OFFSET(motion_img),
	copy_bool,
	print_bool
	},
	{
	"quality",
	"# The quality (in percent) to be used by the jpeg compression (default: 75)",
	CONF_OFFSET(quality),
	copy_int,
	print_int
	},
	{
	"ppm",
	"# Output ppm images instead of jpeg (default: off)",
	CONF_OFFSET(ppm),
	copy_bool,
	print_bool
	},

#ifdef HAVE_FFMPEG
	{
	"ffmpeg_cap_new",
	"\n############################################################\n"
	"# FFMPEG related options\n"
	"# Film (mpeg) file output, and deinterlacing of the video input\n"
	"# The options movie_filename and timelapse_filename are also used\n"
	"# by the ffmpeg feature\n"
	"############################################################\n\n"
	"# Use ffmpeg to encode mpeg movies in realtime (default: off)",
	CONF_OFFSET(ffmpeg_cap_new),
	copy_bool,
	print_bool
	},
	{
	"ffmpeg_cap_motion",
	"# Use ffmpeg to make movies with only the pixels moving\n"
	"# object (ghost images) (default: off)",
	CONF_OFFSET(ffmpeg_cap_motion),
	copy_bool,
	print_bool
	},
	{
	"ffmpeg_timelapse",
	"# Use ffmpeg to encode a timelapse movie\n"
	"# Default value 0 = off - else save frame every Nth second",
	CONF_OFFSET(timelapse),
	copy_int,
	print_int
	},
	{
	"ffmpeg_timelapse_mode",
	"# The file rollover mode of the timelapse video\n"
	"# Valid values: hourly, daily (default), weekly-sunday, weekly-monday, monthly, manual",
	CONF_OFFSET(timelapse_mode),
	copy_string,
	print_string
	},
	{
	"ffmpeg_bps",
	"# Bitrate to be used by the ffmpeg encoder (default: 400000)\n"
	"# This option is ignored if ffmpeg_variable_bitrate is not 0 (disabled)",
	CONF_OFFSET(ffmpeg_bps),
	copy_int,
	print_int
	},
	{
	"ffmpeg_variable_bitrate",
	"# Enables and defines variable bitrate for the ffmpeg encoder.\n"
	"# ffmpeg_bps is ignored if variable bitrate is enabled.\n"
	"# Valid values: 0 (default) = fixed bitrate defined by ffmpeg_bps,\n"
	"# or the range 2 - 31 where 2 means best quality and 31 is worst.",
	CONF_OFFSET(ffmpeg_vbr),
	copy_int,
	print_int
	},
	{
	"ffmpeg_video_codec",
	"# Codec to used by ffmpeg for the video compression.\n"
	"# Timelapse mpegs are always made in mpeg1 format independent from this option.\n"
	"# Supported formats are: mpeg1 (ffmpeg-0.4.8 only), mpeg4 (default), and msmpeg4.\n"
	"# mpeg1 - gives you files with extension .mpg\n"
	"# mpeg4 or msmpeg4 - gives you files with extension .avi\n"
	"# msmpeg4 is recommended for use with Windows Media Player because\n"
	"# it requires no installation of codec on the Windows client.\n"
	"# swf - gives you a flash film with extension .swf\n"
	"# flv - gives you a flash video with extension .flv\n"
	"# ffv1 - FF video codec 1 for Lossless Encoding ( experimental )",
	CONF_OFFSET(ffmpeg_video_codec),
	copy_string,
	print_string
	},
	{
	"ffmpeg_deinterlace",
	"# Use ffmpeg to deinterlace video. Necessary if you use an analog camera\n"
	"# and see horizontal combing on moving objects in video or pictures.\n"
	"# (default: off)",
	CONF_OFFSET(ffmpeg_deinterlace),
	copy_bool,
	print_bool
	},
#endif /* HAVE_FFMPEG */

	{
	"snapshot_interval",
	"\n############################################################\n"
	"# Snapshots (Traditional Periodic Webcam File Output)\n"
	"############################################################\n\n"
	"# Make automated snapshot every N seconds (default: 0 = disabled)",
	CONF_OFFSET(snapshot_interval),
	copy_int,
	print_int
	},

	{
	"locate",
	"\n############################################################\n"
	"# Text Display\n"
	"# %Y = year, %m = month, %d = date,\n"
	"# %H = hour, %M = minute, %S = second, %T = HH:MM:SS,\n"
	"# %v = event, %q = frame number, %t = thread (camera) number,\n"
	"# %D = changed pixels, %N = noise level, \\n = new line,\n"
	"# %i and %J = width and height of motion area,\n"
	"# %K and %L = X and Y coordinates of motion center\n"
	"# %C = value defined by text_event - do not use with text_event!\n"
	"# You can put quotation marks around the text to allow\n"
	"# leading spaces\n"
	"############################################################\n\n"
	"# Locate and draw a box around the moving object.\n"
	"# Valid values: on, off and preview (default: off)\n"
	"# Set to 'preview' will only draw a box in preview_shot pictures.",
	CONF_OFFSET(locate),
	copy_string,
	print_string
	},
	{
	"text_right",
	"# Draws the timestamp using same options as C function strftime(3)\n"
	"# Default: %Y-%m-%d\\n%T = date in ISO format and time in 24 hour clock\n"
	"# Text is placed in lower right corner",
	CONF_OFFSET(text_right),
	copy_string,
	print_string
	},
	{
	"text_left",
	"# Draw a user defined text on the images using same options as C function strftime(3)\n"
	"# Default: Not defined = no text\n"
	"# Text is placed in lower left corner",
	CONF_OFFSET(text_left),
	copy_string,
	print_string
	},
 	{
	"text_changes",
	"# Draw the number of changed pixed on the images (default: off)\n"
	"# Will normally be set to off except when you setup and adjust the motion settings\n"
	"# Text is placed in upper right corner",
	CONF_OFFSET(text_changes),
	copy_bool,
	print_bool
	},
	{
	"text_event",
	"# This option defines the value of the special event conversion specifier %C\n"
	"# You can use any conversion specifier in this option except %C. Date and time\n"
	"# values are from the timestamp of the first image in the current event.\n"
	"# Default: %Y%m%d%H%M%S\n"
	"# The idea is that %C can be used filenames and text_left/right for creating\n"
	"# a unique identifier for each event.",
	CONF_OFFSET(text_event),
	copy_string,
	print_string
	},
	{
	"text_double",
	"# Draw characters at twice normal size on images. (default: off)",
	CONF_OFFSET(text_double),
	copy_bool,
	print_bool
	},

	{
	"target_dir",
	"\n############################################################\n"
	"# Target Directories and filenames For Images And Films\n"
	"# For the options snapshot_, jpeg_, mpeg_ and timelapse_filename\n"
	"# you can use conversion specifiers\n"
	"# %Y = year, %m = month, %d = date,\n"
	"# %H = hour, %M = minute, %S = second,\n"
	"# %v = event, %q = frame number, %t = thread (camera) number,\n"
	"# %D = changed pixels, %N = noise level,\n"
	"# %i and %J = width and height of motion area,\n"
	"# %K and %L = X and Y coordinates of motion center\n"
	"# %C = value defined by text_event\n"
	"# Quotation marks round string are allowed.\n"
	"############################################################\n\n"
	"# Target base directory for pictures and films\n"
	"# Recommended to use absolute path. (Default: current working directory)",
	CONF_OFFSET(filepath),
	copy_string,
	print_string
	},
	{
	"snapshot_filename",
	"# File path for snapshots (jpeg or ppm) relative to target_dir\n"
	"# Default: "DEF_SNAPPATH"\n"
	"# Default value is equivalent to legacy oldlayout option\n"
	"# For Motion 3.0 compatible mode choose: %Y/%m/%d/%H/%M/%S-snapshot\n"
	"# File extension .jpg or .ppm is automatically added so do not include this.\n"
	"# Note: A symbolic link called lastsnap.jpg created in the target_dir will always\n"
	"# point to the latest snapshot, unless snapshot_filename is exactly 'lastsnap'",
	CONF_OFFSET(snappath),
	copy_string,
	print_string
	},
	{
	"jpeg_filename",
	"# File path for motion triggered images (jpeg or ppm) relative to target_dir\n"
	"# Default: "DEF_JPEGPATH"\n"
	"# Default value is equivalent to legacy oldlayout option\n"
	"# For Motion 3.0 compatible mode choose: %Y/%m/%d/%H/%M/%S-%q\n"
	"# File extension .jpg or .ppm is automatically added so do not include this\n"
	"# Set to 'preview' together with best-preview feature enables special naming\n"
	"# convention for preview shots. See motion guide for details",
	CONF_OFFSET(jpegpath),
	copy_string,
	print_string
	},
#ifdef HAVE_FFMPEG
	{
	"movie_filename",
	"# File path for motion triggered ffmpeg films (mpeg) relative to target_dir\n"
	"# Default: "DEF_MPEGPATH"\n"
	"# Default value is equivalent to legacy oldlayout option\n"
	"# For Motion 3.0 compatible mode choose: %Y/%m/%d/%H%M%S\n"
	"# File extension .mpg or .avi is automatically added so do not include this\n"
	"# This option was previously called ffmpeg_filename",
	CONF_OFFSET(mpegpath),
	copy_string,
	print_string
	},
	{
	"timelapse_filename",
	"# File path for timelapse mpegs relative to target_dir\n"
	"# Default: "DEF_TIMEPATH"\n"
	"# Default value is near equivalent to legacy oldlayout option\n"
	"# For Motion 3.0 compatible mode choose: %Y/%m/%d-timelapse\n"
	"# File extension .mpg is automatically added so do not include this",
	CONF_OFFSET(timepath),
	copy_string,
	print_string
	},
#endif /* HAVE_FFMPEG */

	{
	"webcam_port",
	"\n############################################################\n"
	"# Live Webcam Server\n"
	"############################################################\n\n"
	"# The mini-http server listens to this port for requests (default: 0 = disabled)",
	CONF_OFFSET(webcam_port),
	copy_int,
	print_int
	},
	{
	"webcam_quality",
	"# Quality of the jpeg images produced (default: 50)",
	CONF_OFFSET(webcam_quality),
	copy_int,
	print_int
	},
	{
	"webcam_motion",
	"# Output frames at 1 fps when no motion is detected and increase to the\n"
	"# rate given by webcam_maxrate when motion is detected (default: off)",
	CONF_OFFSET(webcam_motion),
	copy_bool,
	print_bool
	},
	{
	"webcam_maxrate",
	"# Maximum framerate for webcam streams (default: 1)",
	CONF_OFFSET(webcam_maxrate),
	copy_int,
	print_int
	},
	{
	"webcam_localhost",
	"# Restrict webcam connections to localhost only (default: on)",
	CONF_OFFSET(webcam_localhost),
	copy_bool,
	print_bool
	},
	{
	"webcam_limit",
	"# Limits the number of images per connection (default: 0 = unlimited)\n"
	"# Number can be defined by multiplying actual webcam rate by desired number of seconds\n"
	"# Actual webcam rate is the smallest of the numbers framerate and webcam_maxrate",
	CONF_OFFSET(webcam_limit),
	copy_int,
	print_int
	},
	{
	"control_port",
	"\n############################################################\n"
	"# HTTP Based Control\n"
	"############################################################\n\n"
	"# TCP/IP port for the http server to listen on (default: 0 = disabled)",
	CONF_OFFSET(control_port),
	copy_int,
	print_int
	},
	{
	"control_localhost",
	"# Restrict control connections to localhost only (default: on)",
	CONF_OFFSET(control_localhost),
	copy_bool,
	print_bool
	},
	{
	"control_html_output",
	"# Output for http server, select off to choose raw text plain (default: on)",
	CONF_OFFSET(control_html_output),
	copy_bool,
	print_bool
	},
	{
	"control_authentication",
	"# Authentication for the http based control. Syntax username:password\n"
	"# Default: not defined (Disabled)",
	CONF_OFFSET(control_authentication),
	copy_string,
	print_string
	},	
	{
	"track_type",
	"\n############################################################\n"
	"# Tracking (Pan/Tilt)\n"
	"############################################################\n\n"
	"# Type of tracker (0=none (default), 1=stepper, 2=iomojo, 3=pwc, 4=generic, 5=uvcvideo)\n"
	"# The generic type enables the definition of motion center and motion size to\n"
	"# be used with the conversion specifiers for options like on_motion_detected",
	TRACK_OFFSET(type),
	copy_int,
	print_int
	},
	{
	"track_auto",
	"# Enable auto tracking (default: off)",
	TRACK_OFFSET(active),
	copy_bool,
	print_bool
	},
	{
	"track_port",
	"# Serial port of motor (default: none)",
	TRACK_OFFSET(port),
	copy_string,
	print_string
	},
	{
	"track_motorx",
	"# Motor number for x-axis (default: -1)",
	TRACK_OFFSET(motorx),
	copy_int,
	print_int
	},
	{
	"track_motory",
	"# Motor number for y-axis (default: -1)",
	TRACK_OFFSET(motory),
	copy_int,
	print_int
	},
	{
	"track_maxx",
	"# Maximum value on x-axis (default: 0)",
	TRACK_OFFSET(maxx),
	copy_int,
	print_int
	},
	 {
	"track_maxy",
	"# Maximum value on y-axis (default: 0)",
	TRACK_OFFSET(maxy),
	copy_int,
	print_int
	},
	{
	"track_iomojo_id",
	"# ID of an iomojo camera if used (default: 0)",
	TRACK_OFFSET(iomojo_id),
	copy_int,
	print_int
	},
	{
	"track_step_angle_x",
	"# Angle in degrees the camera moves per step on the X-axis\n"
	"# with auto-track (default: 10)\n"
	"# Currently only used with pwc type cameras",
	TRACK_OFFSET(step_angle_x),
	copy_int,
	print_int
	},
	{
	"track_step_angle_y",
	"# Angle in degrees the camera moves per step on the Y-axis\n"
	"# with auto-track (default: 10)\n"
	"# Currently only used with pwc type cameras",
	TRACK_OFFSET(step_angle_y),
	copy_int,
	print_int
	},
	{
	"track_move_wait",
	"# Delay to wait for after tracking movement as number\n"
	"# of picture frames (default: 10)",
	TRACK_OFFSET(move_wait),
	copy_int,
	print_int
	},
	{
	"track_speed",
	"# Speed to set the motor to (stepper motor option) (default: 255)",
	TRACK_OFFSET(speed),
	copy_int,
	print_int
	},
	{
	"track_stepsize",
	"# Number of steps to make (stepper motor option) (default: 40)",
	TRACK_OFFSET(stepsize),
	copy_int,
	print_int
	},

	{
	"quiet",
	"\n############################################################\n"
	"# External Commands, Warnings and Logging:\n"
	"# You can use conversion specifiers for the on_xxxx commands\n"
	"# %Y = year, %m = month, %d = date,\n"
	"# %H = hour, %M = minute, %S = second,\n"
	"# %v = event, %q = frame number, %t = thread (camera) number,\n"
	"# %D = changed pixels, %N = noise level,\n"
	"# %i and %J = width and height of motion area,\n"
	"# %K and %L = X and Y coordinates of motion center\n"
	"# %C = value defined by text_event\n"
	"# %f = filename with full path\n"
	"# %n = number indicating filetype\n"
	"# Both %f and %n are only defined for on_picture_save,\n"
	"# on_movie_start and on_movie_end\n" 
	"# Quotation marks round string are allowed.\n"
	"############################################################\n\n"
	"# Do not sound beeps when detecting motion (default: on)\n"
	"# Note: Motion never beeps when running in daemon mode.",
	CONF_OFFSET(quiet),
	copy_bool,
	print_bool
	},
	{
	"on_event_start",
	"# Command to be executed when an event starts. (default: none)\n"
	"# An event starts at first motion detected after a period of no motion defined by gap ",
	CONF_OFFSET(on_event_start),
	copy_string,
	print_string
	},
	{
	"on_event_end",
	"# Command to be executed when an event ends after a period of no motion\n"
	"# (default: none). The period of no motion is defined by option gap.",
	CONF_OFFSET(on_event_end),
	copy_string,
	print_string
	},
	{
	"on_picture_save",
	"# Command to be executed when a picture (.ppm|.jpg) is saved (default: none)\n"
	"# To give the filename as an argument to a command append it with %f",
	CONF_OFFSET(on_picture_save),
	copy_string,
	print_string
	},
	{
	"on_motion_detected",
	"# Command to be executed when a motion frame is detected (default: none)",
	CONF_OFFSET(on_motion_detected),
	copy_string,
	print_string
	},
#ifdef HAVE_FFMPEG
	{
	"on_movie_start",
	"# Command to be executed when a movie file (.mpg|.avi) is created. (default: none)\n"
	"# To give the filename as an argument to a command append it with %f",
	CONF_OFFSET(on_movie_start),
	copy_string,
	print_string
	},
	{
	"on_movie_end",
	"# Command to be executed when a movie file (.mpg|.avi) is closed. (default: none)\n"
	"# To give the filename as an argument to a command append it with %f",
	CONF_OFFSET(on_movie_end),
	copy_string,
	print_string
	},
#endif /* HAVE_FFMPEG */

#if defined(HAVE_MYSQL) || defined(HAVE_PGSQL)
	{
	"sql_log_image",
	"\n############################################################\n"
	"# Common Options For MySQL and PostgreSQL database features.\n"
	"# Options require the MySQL/PostgreSQL options to be active also.\n"
	"############################################################\n\n"
	"# Log to the database when creating motion triggered image file  (default: on)",
	CONF_OFFSET(sql_log_image),
	copy_bool,
	print_bool
	},
	{
	"sql_log_snapshot",
	"# Log to the database when creating a snapshot image file (default: on)",
	CONF_OFFSET(sql_log_snapshot),
	copy_bool,
	print_bool
	},
	{
	"sql_log_mpeg",
	"# Log to the database when creating motion triggered mpeg file (default: off)",
	CONF_OFFSET(sql_log_mpeg),
	copy_bool,
	print_bool
	},
	{
	"sql_log_timelapse",
	"# Log to the database when creating timelapse mpeg file (default: off)",
	CONF_OFFSET(sql_log_timelapse),
	copy_bool,
	print_bool
	},
	{
	"sql_query",
	"# SQL query string that is sent to the database\n"
	"# Use same conversion specifiers has for text features\n"
	"# Additional special conversion specifiers are\n"
	"# %n = the number representing the file_type\n"
	"# %f = filename with full path\n"
	"# Default value:\n"
	"# insert into security(camera, filename, frame, file_type, time_stamp, text_event) values('%t', '%f', '%q', '%n', '%Y-%m-%d %T', '%C')",
	CONF_OFFSET(sql_query),
	copy_string,
	print_string
	},

#endif /* defined(HAVE_MYSQL) || defined(HAVE_PGSQL) */

#ifdef HAVE_MYSQL
	{
	"mysql_db",
	"\n############################################################\n"
	"# Database Options For MySQL\n"
	"############################################################\n\n"
	"# Mysql database to log to (default: not defined)",
	CONF_OFFSET(mysql_db),
	copy_string,
	print_string
	},
	{
	"mysql_host",
	"# The host on which the database is located (default: not defined)",
	CONF_OFFSET(mysql_host),
	copy_string,
	print_string
	},
	{
	"mysql_user",
	"# User account name for MySQL database (default: not defined)",
	CONF_OFFSET(mysql_user),
	copy_string,
	print_string
	},
	{
	"mysql_password",
	"# User password for MySQL database (default: not defined)",
	CONF_OFFSET(mysql_password),
	copy_string,
	print_string
	},
#endif /* HAVE_MYSQL */

#ifdef HAVE_PGSQL
	{
	"pgsql_db",
	"\n############################################################\n"
	"# Database Options For PostgreSQL\n"
	"############################################################\n\n"
	"# PostgreSQL database to log to (default: not defined)",
	CONF_OFFSET(pgsql_db),
	copy_string,
	print_string
	},
	{
	"pgsql_host",
	"# The host on which the database is located (default: not defined)",
	CONF_OFFSET(pgsql_host),
	copy_string,
	print_string
	},
	{
	"pgsql_user",
	"# User account name for PostgreSQL database (default: not defined)",
	CONF_OFFSET(pgsql_user),
	copy_string,
	print_string
	},
	{
	"pgsql_password",
	"# User password for PostgreSQL database (default: not defined)",
	CONF_OFFSET(pgsql_password),
	copy_string,
	print_string
	},
	{
	"pgsql_port",
	"# Port on which the PostgreSQL database is located (default: 5432)",
	CONF_OFFSET(pgsql_port),
	copy_int,
	print_int
	},
#endif /* HAVE_PGSQL */
	
	{
	"video_pipe",
	"\n############################################################\n"
	"# Video Loopback Device (vloopback project)\n"
	"############################################################\n\n"
	"# Output images to a video4linux loopback device\n"
	"# The value '-' means next available (default: not defined)",
	CONF_OFFSET(vidpipe),
	copy_string,
	print_string
	},
	{
	"motion_video_pipe",
	"# Output motion images to a video4linux loopback device\n"
	"# The value '-' means next available (default: not defined)",
	CONF_OFFSET(motionvidpipe),
	copy_string,
	print_string
	},
	{
	"thread",
	"\n##############################################################\n"
	"# Thread config files - One for each camera.\n"
	"# Except if only one camera - You only need this config file.\n"
	"# If you have more than one camera you MUST define one thread\n"
	"# config file for each camera in addition to this config file.\n"
	"##############################################################\n",
	0,
	config_thread,
	print_thread
	},
	{ NULL, NULL, 0 , NULL, NULL }
};

/* conf_cmdline sets the conf struct options as defined by the command line.
 * Any option already set from a config file are overridden.
 */
static void conf_cmdline (struct context *cnt, int thread)
{
	struct config *conf=&cnt->conf;
	int c;

	/* For the string options, we free() if necessary and malloc()
	 * if necessary. This is accomplished by calling mystrcpy();
	 * see this function for more information.
	 */
	while ((c=getopt(conf->argc, conf->argv, "c:d:hns?p:"))!=EOF)
		switch (c) {
			case 'c':
				if (thread==-1) strcpy(cnt->conf_filename, optarg);
				break;
			case 'n':
				cnt->daemon=0;
				break;
			case 's':
				conf->setup_mode=1;
				break;
			case 'd':
				/* no validation - just take what user gives */
				debug_level = atoi(optarg);
                //add by z67625
                debug = 1;
				break;
			case 'p':
				if (thread==-1) strcpy(cnt->pid_file, optarg);
				break;	
			case 'h':
			case '?':
			default:
				usage();
				exit(1);
		}
	optind=1;
}


/* conf_cmdparse sets a config option given by 'cmd' to the value given by 'arg1'.
 * Based on the name of the option it searches through the struct 'config_params'
 * for an option where the config_params[i].param_name matches the option.
 * By calling the function pointed to by config_params[i].copy the option gets
 * assigned.
 */
struct context **conf_cmdparse(struct context **cnt, const char *cmd, const char *arg1)
{
	int i = 0;

	if(!cmd)
		return cnt;

	/* We search through config_params until we find a param_name that matches
	 * our option given by cmd (or reach the end = NULL)
	 */
	while( config_params[i].param_name != NULL ) {
		if(!strncasecmp(cmd, config_params[i].param_name , 255 + 50)) { // Why +50?
	
			/* if config_param is string we don't want to check arg1 */		
			if (strcmp(config_type(&config_params[i]),"string")) {
				if(config_params[i].conf_value && !arg1)
					return cnt;
			}
			
			/* We call the function given by the pointer config_params[i].copy
			 * If the option is a bool, copy_bool is called.
			 * If the option is an int, copy_int is called.
			 * If the option is a string, copy_string is called.
			 * If the option is a thread, config_thread is called.
			 * The arguments to the function are:
			 *  cnt  - a pointer to the context structure
			 *  arg1 - a pointer to the new option value (represented as string)
			 *  config_params[i].conf_value - an integer value which is a pointer
			 *    to the context structure member relative to the pointer cnt.
			 */
			cnt=config_params[i].copy( cnt, arg1, config_params[i].conf_value );
			return cnt;
		}
		i++;
	}

	/* We reached the end of config_params without finding a matching option */
	motion_log(LOG_ERR, 0, "Unknown config option \"%s\"",cmd);

	return cnt;
}

/* conf_process walks through an already open config file line by line
 * Any line starting with '#' or ';' or empty lines are ignored as a comments.
 * Any non empty line is process so that the first word is the name of an option 'cnd'
 * and the rest of the line is the argument 'arg1'
 * White space before the first word, between option and argument and end of the line
 * is discarded. A '=' between option and first word in argument is also discarded.
 * Quotation marks round the argument are also discarded.
 * For each option/argument pair the function conf_cmdparse is called which takes
 * care of assigning the value to the option in the config structures.
 */
static struct context **conf_process(struct context **cnt, FILE *fp)
{
	/* process each line from the config file */
	
	char line[PATH_MAX], *cmd = NULL, *arg1 = NULL;
	char *beg = NULL, *end = NULL;

	while (fgets(line, PATH_MAX-1, fp)) {
		if(!(line[0]=='#' || line[0]==';' || strlen(line)< 2)) {/* skipcomment */
			
			arg1 = NULL;

			/* trim white space and any CR or LF at the end of the line */
			end = line + strlen(line) - 1; /* Point to the last non-null character in the string */
			while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') {
				end--;
			}
			*(end+1) = '\0';
			
			/* If line is only whitespace we continue to the next line */
			if (strlen(line) == 0)
				continue;

			/* trim leading whitespace from the line and find command */
			beg = line;
			while (*beg == ' ' || *beg == '\t') {
				beg++;
			}

			cmd = beg; /* command starts here */

			while (*beg != ' ' && *beg != '\t' && *beg != '=' && *beg != '\0') {
				beg++;
			}
			*beg = '\0'; /* command string terminates here */

			/* trim space between command and argument */
			beg++;

			if (strlen(beg) > 0){
				while (*beg == ' ' || *beg == '\t' || *beg == '=' || *beg == '\n' || *beg == '\r') {
					beg++;
				}

				/* If argument is in "" we will strip them off
				   It is important that we can use "" so that we can use
				   leading spaces in text_left and text_right */
				if ((beg[0]=='"' && beg[strlen(beg)-1]=='"') ||
				    (beg[0]=='\'' && beg[strlen(beg)-1]=='\'')) {
					beg[strlen(beg)-1]='\0';
					beg++;
				}
				
				arg1 = beg; /* Argument starts here */
			}
			/* else arg1 stays null pointer */

			cnt = conf_cmdparse(cnt, cmd, arg1);
		}
	}

	return cnt;
}


/* conf_print is used to write out the config file(s) motion.conf and any thread
 * config files. The function is called when using http remote control.
 */
void conf_print(struct context **cnt)
{
	const char *retval;
	char *val;
	int i, thread;
	FILE *conffile;

	for (thread=0; cnt[thread]; thread++) {
		motion_log(LOG_INFO, 1, "Writing config file to %s",cnt[thread]->conf_filename);
		conffile=myfopen(cnt[thread]->conf_filename, "w");
		if (!conffile)
			continue;
		fprintf(conffile, "# %s\n", cnt[thread]->conf_filename);
		fprintf(conffile, "#\n# This config file was generated by motion " VERSION "\n");
		fprintf(conffile, "\n\n");
		for (i=0; config_params[i].param_name; i++) {
			retval=config_params[i].print(cnt, NULL, i, thread);
			/*If config parameter has a value (not NULL) print it to the config file*/
			if (retval) {
				fprintf(conffile, "%s\n", config_params[i].param_help);
				/* If the option is a text_* and first char is a space put
				   quotation marks around to allow leading spaces */
				if (strncmp(config_params[i].param_name, "text", 4) || strncmp(retval, " ", 1))
					fprintf(conffile, "%s %s\n\n", config_params[i].param_name, retval);
				else
					fprintf(conffile, "%s \"%s\"\n\n", config_params[i].param_name, retval);
			} else {
				val = NULL;
				config_params[i].print(cnt, &val, i, thread);
				/* It can either be a thread file parameter or a disabled parameter
				   If it is a thread parameter write it out
				   Else write the disabled option to the config file but with a
				   comment mark in front of the parameter name */
				if (val) {
					fprintf(conffile,"%s\n", config_params[i].param_help);
					fprintf(conffile, "%s\n", val);
					if (strlen(val)==0)
						fprintf(conffile,"; thread /usr/local/etc/thread1.conf\n");
					free(val);
				} else if (thread==0) {		
					fprintf(conffile,"%s\n", config_params[i].param_help);
					fprintf(conffile,"; %s value\n\n", config_params[i].param_name);
				}
			}
		}

		fprintf(conffile, "\n");
		fclose(conffile);
		conffile=NULL;
	}
}

//add by z67625
//获取lan侧配置文件
void get_lanconf(void)
{
    FILE *fp = NULL;
    char acLine[300];
    char *p = NULL;
    int i = 0;
    if(NULL == (fp = fopen("/var/udhcpd/udhcpd.conf","r")))
    {
        MDEBUG("open /var/udhcpd/udhcpd.conf failed\n");
        return 0;
    }
    memset(acLine,0,sizeof(acLine));
    while(fgets(acLine,300,fp))
    {
		if (strchr(acLine, '\n')) *(strchr(acLine, '\n')) = '\0';
		if (strchr(acLine, '#')) *(strchr(acLine, '#')) = '\0';
        if(NULL != (p = strchr(acLine, ' '))) 
        {
            *p = '\0';
            p++;
        }
        if(strcmp(acLine,"start") == 0)
        {
            strncpy(lanIp[0],p,16);
        }
        if(strcmp(acLine,"end") == 0)
        {
            strncpy(lanIp[1],p,16);
        }
        memset(acLine,0,sizeof(acLine));
    }
    fclose(fp);
}
//获取用户配置文件
struct context ** get_userconf(struct context **cnt, int flag)
{
    FILE *fp = NULL;
    char acLine[128] = "";
    char buf[512] = "";
    char *p = NULL;
    int i = 0;
    int resetflag = 0;

    MLOG("\nbegin to get user conf \n");
    //system("echo > /var/motion/curcfg.conf");
    //sprintf(buf,"enable=1\nauto_brightness=0\nbrightness=75\nnetcam_url=\nnetcam_userpass=Huawei:huawei\nport=18080\nframerate=3\nremote_enable=1\nremote_pass=123:789\nwidth=320\nheight=240");
    //fp = fopen(CONFPATH,"w");
    //fputs(buf,fp);
    //fclose(fp);
    //fp = NULL;
    fp = fopen(CONFPATH,"r");
    if(fp)
    {
        while(fgets(acLine,128,fp))
        {
            //去掉'\n'
            if(NULL != (p = strstr(acLine,"\n")))
            {
                *p = '\0';
            }
            //auto_brightness
            if(NULL != (p = strstr(acLine,"auto_brightness=")))
            {
                p += strlen("auto_brightness=");
                if(atoi(p)!= 0 && atoi(p) != 1)
                {
                    printf("auto_brightness value %d is error\n",atoi(p));
                    continue;
                }
                MLOG("auto_brightness value set to %d\n",atoi(p)); 
                if(cnt[0]!=NULL)
                    cnt[0]->conf.autobright = atoi(p);
               
            }
            //brightness
            else if(NULL != (p = strstr(acLine,"brightness=")))
            {
                p += strlen("brightness=");
                if((atoi(p) > 255) ||(atoi(p) < 0))
                {
                    printf("\nbrightness value %d is error \n",atoi(p));
                    continue;
                }
                MLOG("brightness value set to %d\n",atoi(p));
                if(cnt[0]!=NULL)
                {
                    cnt[0]->conf.brightness = atoi(p);
                }
            }
            //width
            else if(NULL != (p = strstr(acLine,"width=")))
            {
                p += strlen("width=");
                //长宽修改了必须重启
                if(!flag && cnt[0]!=NULL && cnt[0]->conf.width != atoi(p))
                {
                    restart = 1;
                }
                MLOG("width value set to %d\n",atoi(p));
                if(cnt[0]!=NULL)
                {
                    cnt[0]->conf.width = atoi(p);
                }
            }
            //height
            else if(NULL != (p = strstr(acLine,"height=")))
            {
                p += strlen("height=");
                MLOG("height value set to %d\n",atoi(p));
                if(cnt[0]!=NULL)
                {
                    cnt[0]->conf.height = atoi(p);
                }
            }
            //netcam_url
            else if(NULL != (p = strstr(acLine,"netcam_url=")))
            {
                p +=strlen("netcam_url=");
                if(strlen(p) == 0 && cnt[0]!=NULL && cnt[0]->conf.netcam_url == NULL)
                {
                    continue;
                }
                else if(strlen(p)!=0 && cnt[0]!=NULL && cnt[0]->conf.netcam_url != NULL)
                {
                    if(strcasecmp(p,cnt[0]->conf.netcam_url))
                    {
                        if(!flag)
                            restart = 1;
                        MLOG("netcam_url value set to %s\n",p);                 
                        free(cnt[0]->conf.netcam_url);
                        if(NULL != (cnt[0]->conf.netcam_url = (char *)malloc(strlen(p)+1)))
                        {
                            strcpy(cnt[0]->conf.netcam_url ,p);
                        }
                    }
                }
                else if(strlen(p)!=0 && cnt[0]!=NULL && cnt[0]->conf.netcam_url == NULL)
                {
                    if(!flag)
                        restart = 1;
                    MLOG("netcam_url value set to %s\n",p);
                    if(NULL != (cnt[i]->conf.netcam_url = (char *)malloc(strlen(p)+1)))
                    {
                        strcpy(cnt[i]->conf.netcam_url ,p);
                    }    
                }
                else if(cnt[0]!=NULL)
                {
                    if(!flag)
                        restart = 1;
                    MLOG("netcam_url value set to NULL\n");
                    if(cnt[0]->conf.netcam_url != NULL)
                    {
                        free(cnt[0]->conf.netcam_url);
                    }
                    cnt[i]->conf.netcam_url = NULL;
                }
            }
            //netcam_userpass
            else if(NULL != (p = strstr(acLine,"netcam_userpass=")))
            {
                p += strlen("netcam_userpass=");
                if(strlen(p) == 0 && cnt[0]!=NULL && cnt[0]->conf.netcam_userpass == NULL)
                {
                    continue;
                }
                else if(strlen(p)!=0 && cnt[0]!=NULL && cnt[0]->conf.netcam_userpass != NULL)
                {
                    MLOG("netcam_userpass value set to %s\n",p);
                    if(strcasecmp(p,cnt[0]->conf.netcam_userpass))
                    {
                        free(cnt[0]->conf.netcam_userpass);
                        if(NULL != (cnt[0]->conf.netcam_userpass = (char *)malloc(strlen(p)+1)))
                        {
                            strcpy(cnt[0]->conf.netcam_userpass ,p);
                        }
                    }
                }
                else if(strlen(p)!=0 && cnt[0]!=NULL && cnt[0]->conf.netcam_userpass == NULL)
                {
                    MLOG("netcam_userpass value set to %s\n",p);
                    if(NULL != (cnt[0]->conf.netcam_userpass = (char *)malloc(strlen(p)+1)))
                    {
                        strcpy(cnt[0]->conf.netcam_userpass ,p);
                    }

                }
                else if(cnt[0]!=NULL)
                {
                    MLOG("netcam_userpass value set to NULL\n"); 
                    if(cnt[0]->conf.netcam_userpass != NULL)
                    {
                        free(cnt[0]->conf.netcam_userpass);
                    }
                    cnt[0]->conf.netcam_userpass = NULL;

                }
            }
            //port
            else if(NULL != (p = strstr(acLine,"port=")))
            {
                p+= strlen("port=");
                if(!flag && cnt[0]!=NULL && cnt[0]->conf.webcam_port!= atoi(p))
                {
                    MLOG("port change old is %d,new is %d\n",cnt[0]->conf.webcam_port,atoi(p));
                    restart = 1;
                }
                MLOG("port value set to %d\n",atoi(p));
                if(cnt[0]!=NULL)
                    cnt[0]->conf.webcam_port = atoi(p);

            }
            //framerate
            else if(NULL != (p = strstr(acLine,"framerate=")))
            {
                p+=strlen("framerate=");
                MLOG("framerate set to %d\n",atoi(p));
                if(cnt[0]!=NULL)
                {
                    cnt[0]->conf.frame_limit = atoi(p);
                }
            }
            //remote_enable
            else if(NULL != (p = strstr(acLine,"remote_enable=")))
            {
                p+= strlen("remote_enable=");
                remote_enable = atoi(p);
                MLOG("remote_enable value set to %d\n",atoi(p));
            }
            //remote_pass
            else if(NULL != (p = strstr(acLine,"remote_pass=")))
            {
                p += strlen("remote_pass=");
                if(strlen(p) > 0)
                {
                    MLOG("remote_pass value set to %s\n",p);
                    if(NULL != remote_pass)
                    {
                        free(remote_pass);
                    }
                    if(NULL != (remote_pass = (char *)malloc(strlen(p)+1)))
                    {
                        strcpy(remote_pass,p);
                    }
                }
                else
                {
                    MLOG("remote_pass value set to NULL\n");
                    if(NULL != remote_pass)
                    {
                        free(remote_pass);
                    }
                    remote_pass = NULL;
                }
            }
        }
        fclose(fp);
        if(debug && cnt[0]!=NULL)
        {
            printf("\r\nauto_brightness is [%d]\n",cnt[0]->conf.autobright);
            printf("brightness is [%d]\n",cnt[0]->conf.brightness);
            printf("width is [%d]\n",cnt[0]->conf.width);
            printf("height is [%d]\n",cnt[0]->conf.height);
            if(cnt[0]->conf.netcam_url != NULL)
            {
                printf("netcam_url is [%s]\n",cnt[0]->conf.netcam_url);
            }
            else
                printf("netcam_url is NULL\n");
            if(cnt[0]->conf.netcam_userpass!= NULL)
                printf("netcam_userpass is %s\n",cnt[0]->conf.netcam_userpass);
            else
                printf("netcam_userpass is NULL\n");
            printf("port is %d\n",cnt[0]->conf.webcam_port);
            printf("framerate is %d\n",cnt[0]->conf.frame_limit);
            printf("remote_enable is %d\n",remote_enable);
            if(remote_pass != NULL)
                printf("remote_pass is %s\n",remote_pass);
            else
                printf("remote_pass is NULL\n");
        }
    }
    else
    {
        MLOG("\nopen %s error\n",CONFPATH);
    }
    return cnt;
        
}
//add by z67625

/**************************************************************************
 * conf_load is the main function, called from motion.c
 * The function sets the important context structure "cnt" including
 * loading the config parameters from config files and command line.
 * The following takes place in the function:
 * - The default start values for cnt stored in the struct conf_template
 *   are copied to cnt[0] which is the default context structure common to
 *   all threads.
 * - All config (cnt.conf) struct members pointing to a string are changed
 *   so that they point to a malloc'ed piece of memory containing a copy of
 *   the string given in conf_template.
 * - motion.conf is opened and processed. The process populates the cnt[0] and
 *   for each thread config file it populates a cnt[1], cnt[2]... for each
 *   thread
 * - Finally it process the options given in the command line. This is done
 *   for each thread cnt[i] so that the command line options overrides any
 *   option given by motion.conf or a thread config file.
 **************************************************************************/
struct context ** conf_load (struct context **cnt)
{
	FILE *fp=NULL;
	char filename[PATH_MAX];
	int i;
	/* We preserve argc and argv because they get overwritten by the memcpy command */
	char **argv = cnt[0]->conf.argv;
	int argc = cnt[0]->conf.argc;

	/* Copy the template config structure with all the default config values
	 * into cnt[0]->conf
	 */
	memcpy(&cnt[0]->conf, &conf_template, sizeof(struct config));//赋初值
	
	/* For each member of cnt[0] which is a pointer to a string
	 * if the member points to a string in conf_template and is not NULL
	 * 1. Reserve (malloc) memory for the string
	 * 2. Copy the conf_template given string to the reserved memory
	 * 3. Change the cnt[0] member (char*) pointing to the string in reserved memory
	 * This ensures that we can free and malloc the string when changed
	 * via http remote control or config file or command line options
	 */
	malloc_strings(cnt[0]);

	/* Restore the argc and argv */
	cnt[0]->conf.argv = argv;
	cnt[0]->conf.argc = argc;

	/* Open the motion.conf file. We try in this sequence:
	 * 1. commandline
	 * 2. current working directory
	 * 3. $HOME/.motion/motion.conf
	 * 4. sysconfig/motion.conf
	 */
	/* Get filename & pid file from commandline */
	cnt[0]->conf_filename[0] = 0;
	cnt[0]->pid_file[0] = 0;

	conf_cmdline(cnt[0], -1);
	if (cnt[0]->conf_filename[0]){ /* User has supplied filename on commandline*/
		strcpy(filename, cnt[0]->conf_filename);
		fp = fopen (filename, "r");
	}
	if (!fp){      /* Commandline didn't work, try current dir */
		if (cnt[0]->conf_filename[0])
			motion_log(-1, 1, "Configfile %s not found - trying defaults.", filename);
		sprintf(filename, "motion.conf");
		fp = fopen (filename, "r");
	}
	if (!fp) {     /* specified file does not exist... try default file */
		snprintf(filename, PATH_MAX, "%s/.motion/motion.conf", getenv("HOME"));
		fp = fopen(filename, "r");
		if (!fp) {
			snprintf(filename, PATH_MAX, "%s/motion.conf", sysconfdir);
			fp = fopen(filename, "r");
			if (!fp)        /* there is no config file.... use defaults */
				motion_log(-1, 1, "could not open configfile %s",filename);
		}
	}

	/* Now we process the motion.conf config file and close it*/
	if (fp) {
		strcpy(cnt[0]->conf_filename, filename);
		motion_log(LOG_INFO, 0, "Processing thread 0 - config file %s",filename);
		cnt=conf_process(cnt, fp);
		fclose(fp);
	}else{
		motion_log(LOG_INFO, 0, "Not config file to process using default values");
	}
	

	/* For each thread (given by cnt[i]) being not null
	 * cnt is an array of pointers to a context type structure
	 * cnt[0] is the default context structure
	 * cnt[1], cnt[2], ... are context structures for each thread
	 * Command line options always wins over config file options
	 * so we go through each thread and overrides any set command line
	 * options
	 */
	i=-1;
	while(cnt[++i])
		conf_cmdline(cnt[i], i);

	/* if pid file was passed from command line copy to main thread conf struct */
	if (cnt[0]->pid_file[0])
		cnt[0]->conf.pid_file = mystrcpy(cnt[0]->conf.pid_file, cnt[0]->pid_file);

	return cnt;
}

/* malloc_strings goes through the members of a context structure.
 * For each context structure member which is a pointer to a string it does this:
 * If the member points to a string and is not NULL
 * 1. Reserve (malloc) memory for the string
 * 2. Copy the original string to the reserved memory
 * 3. Change the cnt member (char*) pointing to the string in reserved memory
 * This ensures that we can free and malloc the string if it is later changed
 */
void malloc_strings (struct context * cnt)
{
	int i = 0;
	char **val;
	while( config_params[i].param_name != NULL ) {
		if (config_params[i].copy == copy_string) { /* if member is a string */
			/* val is made to point to a pointer to the current string */
			val = (char **)((char *)cnt+config_params[i].conf_value);

			/* if there is a string, malloc() space for it, copy
			 * the string to new space, and point to the new
			 * string. we don't free() because we're copying a
			 * static string.
			 */
			*val = mystrdup(*val);
		}
		i++;
	}
}

/************************************************************************
 * copy functions
 *
 *   copy_bool   - convert a bool representation to int
 *   copy_int    - convert a string to int
 *   copy_string - just a string copy
 *
 * @param str     - A char *, pointing to a string representation of the
 *                  value.
 * @param val_ptr - points to the place where to store the value relative
 *                  to pointer pointing to the given context structure
 * @cnt           - points to a context structure for a thread
 *
 * The function is given a pointer cnt to a context structure and a pointer val_ptr
 * which is an integer giving the position of the structure member relative to the
 * pointer of the context structure.
 * If the context structure is for thread 0 (cnt[0]->threadnr is zero) then the
 * function also sets the value for all the child threads since thread 0 is the
 * global thread.
 * If the thread given belongs to a child thread (cnt[0]->threadnr is not zero)
 * the function will only assign the value for the given thread.
 ***********************************************************************/

/* copy_bool assigns a config option to a new boolean value.
 * The boolean is given as a string in str which is converted to 0 or 1 
 * by the function. Values 1, yes and on are converted to 1 ignoring case.
 * Any other value is converted to 0.
 */
static struct context **copy_bool (struct context **cnt, const char *str, int val_ptr)
{
	void *tmp;
	int i;

	i=-1;
	while(cnt[++i]) {
		tmp = (char *)cnt[i]+(int)val_ptr;
		if ( !strcmp(str, "1") || !strcasecmp(str, "yes") || !strcasecmp(str,"on")) {
			*((int *)tmp) = 1;
		} else {
			*((int *)tmp) = 0;
		}
		if (cnt[0]->threadnr)
			return cnt;
	}
	return cnt;
}

/* copy_int assigns a config option to a new integer value.
 * The integer is given as a string in str which is converted to integer by the function.
 */
static struct context ** copy_int(struct context **cnt, const char *str, int val_ptr)
{
	void *tmp;
	int i;

	i=-1;
	while(cnt[++i]) {
		tmp = (char *)cnt[i]+val_ptr;
		*((int *)tmp) = atoi(str);
		if (cnt[0]->threadnr)
			return cnt;
	}
	return cnt;
}

/* copy_string assigns a new string value to a config option.
 * Strings are handled differently from bool and int.
 * the char *conf->option that we are working on is free()'d
 * (if memory for it has already been malloc()'d), and set to
 * a freshly malloc()'d string with the value from str,
 * or NULL if str is blank
 */
struct context **copy_string(struct context **cnt, const char *str, int val_ptr)
{
	char **tmp;
	int i;

	i=-1;
	while(cnt[++i]) {
		tmp = (char **)((char *)cnt[i] + val_ptr);

		/* mystrcpy assigns the new string value
		 * including free'ing and reserving new memory for it.
		 */
		*tmp = mystrcpy(*tmp, str);

		/* set the option on all threads if setting the option
		 * for thread 0; otherwise just set that one thread's option
		 */
		if (cnt[0]->threadnr)
			return cnt;
	}
	return cnt;
}


/* mystrcpy is used to assign string type fields (e.g. config options)
 * In a way so that we the memory is malloc'ed to fit the string.
 * If a field is already pointing to a string (not NULL) the memory of the
 * old string is free'd and new memory is malloc'ed and filled with the
 * new string is copied into the the memory and with the char pointer
 * pointing to the new string.
 *
 * from - pointer to the new string we want to copy
 * to   - the pointer to the current string (or pointing to NULL)
 *        If not NULL the memory it points to is free'd.
 * function returns pointer to the new string which is in malloc'ed memory
 * FIXME The strings that are malloc'ed with this function should be freed
 * when the motion program is terminated normally instead of relying on the
 * OS to clean up.
 */
char *mystrcpy(char *to, const char *from)
{
	/* free the memory used by the to string, if such memory exists,
	 * and return a pointer to a freshly malloc()'d string with the
	 * same value as from.
	 */

	if (to != NULL) 
		free(to);

	return mystrdup(from);
}


/* mystrdup return a pointer to a freshly malloc()'d string with the same
 * value as the string that the input parameter 'from' points to,
 * or NULL if the from string is 0 characters.
 * The function truncates the string to the length given by the environment
 * variable PATH_MAX to ensure that config options can always contain
 * a really long path but no more than that.
 */
char *mystrdup(const char *from)
{
	char *tmp;
	int stringlength;

	if (from == NULL || !strlen(from)) {
		tmp = NULL;
	} else {
		stringlength = strlen(from);
		stringlength = (stringlength < PATH_MAX ? stringlength : PATH_MAX);
		tmp = (char *)mymalloc(stringlength + 1);
		strncpy(tmp, from, stringlength);

		/* We must ensure the string always has a NULL terminator.
		 * This necessary because strncpy will not append a NULL terminator
		 * if the original string is greater than stringlength.
		 */
		tmp += stringlength;
		*tmp = '\0';
		tmp -= stringlength;
	}
	return tmp;
}

const char *config_type(config_param *configparam)
{
	if (configparam->copy == copy_string)
		return "string";
	if (configparam->copy == copy_int)
		return "int";
	if (configparam->copy == copy_bool)
		return "bool";
	return "unknown";
}

static const char *print_bool(struct context **cnt, char **str ATTRIBUTE_UNUSED,
                              int parm, int threadnr)
{
	int val=config_params[parm].conf_value;

	if (threadnr &&
	    *(int*)((char *)cnt[threadnr] + val) == *(int*)((char *)cnt[0] + val))
		return NULL;

	if (*(int*)((char *)cnt[threadnr] + val))
		return "on";
	else
		return "off";
}

/* print_string returns a pointer to a string containing the value of the config option
 * If the option is not defined NULL is returned.
 * If the thread number is not 0 the string is compared with the value of the same
 * option in thread 0. If the value is the same, NULL is returned which means that
 * the option is not written to the thread config file.
 */
static const char *print_string(struct context **cnt,
                                char **str ATTRIBUTE_UNUSED, int parm,
                                int threadnr)
{
	int val=config_params[parm].conf_value;
	const char **cptr0, **cptr1;
	
	/* strcmp does not like NULL so we have to check for this also */
	cptr0 = (const char **)((char *)cnt[0] + val);
	cptr1 = (const char **)((char *)cnt[threadnr] + val);
	if ((threadnr) && (*cptr0 != NULL) && (*cptr1 != NULL) && (!strcmp(*cptr0, *cptr1)))
		return NULL;

	return *cptr1;
}

static const char *print_int(struct context **cnt, char **str ATTRIBUTE_UNUSED,
                             int parm, int threadnr)
{
	static char retval[20];
	int val = config_params[parm].conf_value;

	if (threadnr &&
	    *(int*)((char *)cnt[threadnr] + val) == *(int*)((char *)cnt[0] + val))
		return NULL;

	sprintf(retval, "%d", *(int*)((char *)cnt[threadnr] + val));

	return retval;
}

static const char *print_thread(struct context **cnt, char **str,
                                int parm ATTRIBUTE_UNUSED, int threadnr)
{
	char *retval;
	int i=0;

	if (!str || threadnr)
		return NULL;

	retval = mymalloc(1);
	retval[0] = 0;
	while (cnt[++i]) {
		retval = myrealloc(retval, strlen(retval) + strlen(cnt[i]->conf_filename)+10, "print_thread");
		sprintf(retval + strlen(retval), "thread %s\n", cnt[i]->conf_filename);
	}
	*str = retval;

	return NULL;
}

/* config_thread() is called during initial config file loading each time Motion
 * finds a thread option in motion.conf
 * The size of the context array is increased and the main context's values are
 * copied to the new thread.
 *
 * cnt  - pointer to the array of pointers pointing to the context structures
 * str  - pointer to a string which is the filename of the thread config file
 * val  - is not used. It is defined to be function header compatible with
 *        copy_int, copy_bool and copy_string.
 */
static struct context **config_thread(struct context **cnt, const char *str,
                                      int val ATTRIBUTE_UNUSED)
{
	int i;
	FILE *fp;
	
	if (cnt[0]->threadnr)
		return cnt;

	fp=fopen(str, "r");
	if (!fp) {
		motion_log(LOG_ERR, 1, "Thread config file %s not found",str);
		return cnt;
	}

	/* Find the current number of threads defined. */
	i=-1;
	while (cnt[++i]);

	/* Make space for the threads + the terminating NULL pointer
	 * in the array of pointers to context structures
	 * First thread is 0 so the number of threads is i+1
	 * plus an extra for the NULL pointer. This gives i+2
	 */
	cnt = myrealloc(cnt, sizeof(struct context *)*(i+2), "config_thread");

	/* Now malloc space for an additional context structure for thread nr. i */
	cnt[i] = mymalloc(sizeof(struct context));
	
	/* And make this an exact clone of the context structure for thread 0 */
	memcpy(cnt[i], cnt[0], sizeof(struct context));

	/* All the integers are copies of the actual value.
	 * The strings are all pointers to strings so we need to create
	 * unique malloc'ed space for all the strings that are not NULL and
	 * change the string pointers to point to the new strings.
	 * malloc_strings takes care of this.
	 */
	malloc_strings(cnt[i]);
	
	/* Mark the end if the array of pointers to context structures */
	cnt[i+1] = NULL;

	/* process the thread's config file and notify user on console */
	strcpy(cnt[i]->conf_filename, str);
	motion_log(LOG_INFO, 0, "Processing config file %s", str);
	conf_process(cnt+i, fp);
	
	/* Finally we close the thread config file */
	fclose(fp);

	return cnt;
}

static void usage ()
{
	printf("motion Version "VERSION", Copyright 2000-2005 Jeroen Vreeken/Folkert van Heusden/Kenneth Lavrsen\n");
	printf("\nusage:\tmotion [options]\n");
	printf("\n\n");
	printf("Possible options:\n\n");
	printf("-n\t\t\tRun in non-daemon mode.\n");
	printf("-s\t\t\tRun in setup mode.\n");
	printf("-c config\t\tFull path and filename of config file.\n");
	printf("-d level\t\tDebug mode.\n");
	printf("-p process_id_file\tFull path and filename of process id file (pid file).\n");
	printf("-h\t\t\tShow this screen.\n");
	printf("\n");
	printf("Motion is configured using a config file only. If none is supplied,\n");
	printf("it will read motion.conf from current directory, ~/.motion or %s.\n", sysconfdir);
	printf("\n");
	
}
