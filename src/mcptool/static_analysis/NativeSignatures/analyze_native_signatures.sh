#!/bin/bash

## WHEN NOT RUN OVER ADB (for example over termux)

## output colors ##
RED='\033[0;31m'
YELLOW='\033[0;33m'
GREEN='\033[0;32m'
NC='\033[0m'
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Allow callers (e.g., MCP wrappers) to override the target_APK directory.
# If not provided, fall back to the repo-local target_APK folder.
if [ -n "${TARGET_APK_DIR:-}" ]; then
	TARGET_APK_PATH="$TARGET_APK_DIR"
else
	TARGET_APK_PATH="$SCRIPT_DIR/../../target_APK"
fi

# help option
Help()
{
   # Display Help
   echo "Syntax: ./analyze_native_signatures [-h] appname"
   echo
   echo "Analyze APK native methods signatures, group by and count them"
   echo
   echo "Requirements:"
   echo "   - target_APK folder structure:"
   echo "      ├── target_APK/"
   echo "      │   ├── appname/"
   echo "      │   │	  └── base.apk"
   echo "      │   └── ..."
   echo "	   |   └── analyzed.txt "
   echo
   echo "Options:"
   echo "   -h, --help     Print this Help."
   echo "   appname		   The name of the app to be analyzed"
   echo
   echo "Environment:"
   echo "   TARGET_APK_DIR  Override the base target_APK directory (default: <repo>/target_APK)"
   echo "                  Example: TARGET_APK_DIR=/path/to/target_APK ./analyze_native_signatures.sh appname"
   echo
}


Analyze()
{

	TARGET_APPNAME=$1
	TARGET_DIR=$TARGET_APK_PATH"/"$TARGET_APPNAME

	APP_NAME="$TARGET_APPNAME"

	if [ -n "${CURRENT_NUM_APK:-}" ] && [ -n "${TOTAL_NUM_APK:-}" ]; then
		echo -e "${GREEN}[LOG]${NC} Analyzing $APP_NAME ($CURRENT_NUM_APK/$TOTAL_NUM_APK)"
	else
		echo -e "${GREEN}[LOG]${NC} Analyzing $APP_NAME"
	fi

		## extract /lib folder (for harness usage later on) ##

	cp "$TARGET_DIR/base.apk" "$TARGET_DIR/base.zip"
	unzip -o -q "$TARGET_DIR/base.zip" -d "$TARGET_DIR/base_apk"
	if [ -d "$TARGET_DIR/base_apk/lib/arm64-v8a" ] ; then
		rm -rf "$TARGET_DIR/lib"
		mv "$TARGET_DIR/base_apk/lib/" "$TARGET_DIR"
	elif [ -d "$TARGET_DIR/base_apk/lib/arm64" ] ; then
		mv "$TARGET_DIR/base_apk/lib/arm64" "$TARGET_DIR/base_apk/lib/arm64-v8a"
		rm -rf "$TARGET_DIR/lib"
		mv "$TARGET_DIR/base_apk/lib/" "$TARGET_DIR"
	else
		echo -e "${RED}[ERR]${NC} App $APP_NAME either not native or APK doesn't provide arm64 version of libraries"
		((CURRENT_NUM_APK=CURRENT_NUM_APK+1))
		rm "$TARGET_DIR/base.zip"
		rm -rf "$TARGET_DIR/base_apk"
		return
	fi
	rm "$TARGET_DIR/base.zip"
	rm -rf "$TARGET_DIR/base_apk"

	## decompile apk ##
		
	echo -e "${GREEN}[LOG]${NC} Decompiling $APP_NAME"

	jadx --no-res --show-bad-code -d "$TARGET_DIR/base" "$TARGET_DIR/base.apk"

	## exctract native methods ##

	echo -e "${GREEN}[LOG]${NC} Extracting native methods from $APP_NAME"

	# loop until exctracted correctly
	cd "$SCRIPT_DIR/extractor_pattern"
	FLAG=true
	while [ "$FLAG" = true ] ; do
		# extract using qdox
		EXTRACT_OUT=$(java -cp 'com.qdox.jar:.' extractor "$TARGET_DIR" 2>&1)
		FAILURE="Exception in thread"
		#if [ $? -eq 0 ] ; then
		# if successful, exit
		#	echo -e "$EXTRACT_OUT"
		#	FLAG=false
		# Check if not succesfull, for me the java qdox command always returns 0
		if [[ "$EXTRACT_OUT" != *"$FAILURE"* ]]; then
			echo -e "$EXTRACT_OUT"
			FLAG=false
		else
			# if present file qdox can't parse, remove and keep trying
			# get buggy filename
			EXTRACT_OUT=$(awk 'NR > 2 { print }' <<< "$EXTRACT_OUT")
			#echo "file after first awk ${EXTRACT_OUT}"
			ERROR_FILE=$(awk '{ sub(/.*file:/, ""); sub(/\n.*at */, ""); print $1}' <<< "$EXTRACT_OUT" | head -n 1)
			# safety check (prevent any unpleasent behaviour)
			#echo "removing file (after second awk) ${ERROR_FILE}"
			if [[ $ERROR_FILE == *"$TARGET_DIR"*  ]]; then
				echo -e "${YELLOW}[WRN]${NC} Removing buggy file $ERROR_FILE"
				# remove it
				rm $ERROR_FILE
			fi
		fi
	done
	cd ../..

	rm -r "$TARGET_DIR/base"

	echo -e "${GREEN}[LOG]${NC} Done for $APP_NAME"
}

# Main
if [ "$#" -eq 0 ]; then
    echo "Try './analyze_native_signatures.sh --help' for more information."
    exit 1
elif [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    Help
    exit 0
elif [[ "$#" -le 0 ]]; then
    echo "Error usage..."
    Help
    exit 1
else
    Analyze $1
    exit 0
fi
