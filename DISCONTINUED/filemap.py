import os, shutil, sys

root_dir = "."

tokens = [
	['JSString','TiString'],
	['JavaScript','Ti'],
	['JSRetain','TiRetain'],
	['JSRelease','TiRelease'],
	['JSObject','TiObject'],
	['JSLock','TiLock'],
	['JSUnlock','TiUnlock'],
	['JSCell','TiCell'],
	['JSClass','TiClass'],
	['JSStatic','TiStatic'],
	['JSContext','TiContext'],
	['JSGlobal','TiGlobal'],
	['JSValue','TiValue'],
	['JSArray','TiArray'],
	['JSByte','TiArray'],
	['JSFunction','TiFunction'],
	['JSProperty','TiProperty'],
	['ExecState','TiExcState'],
	['JSGlobalData','TiGlobalData'],
	['kJS','kTI'],
	['JSEvaluate','TiEval'],
	['JSCheck','TiCheck'],
	['JSGarbage','TiGarbage'],
	['JSType','TiType'],
	['JSAPI','TiAPI'],
	['JSCallback','TiCallback'],
	['JSProfile','TiProfile'],
	['JSBase','TiBase'],
	['JSChar','TiChar'],
	['WTFMain','WTIMain'],
]

filemap = dict()

def map_filename(fn):
	dirname = os.path.dirname(fn)
	path = os.path.basename(fn)
	ext = os.path.splitext(path)[1]
	if ext in ('.c','.cpp','.mm','.h','.pbxproj','.exp','.xcconfig','.sh','.make','.y','.lut.h') or path == 'create_hash_table':
		for token in tokens:
			if token[0] in path:
				filemap[path.replace(token[0], token[1])] = path
				
for root, dirs, files in os.walk(os.path.abspath(root_dir)):
	for f in files:
		map_filename(os.path.join(root, f))
		
proj_file = "JavaScriptCore.xcodeproj/project.pbxproj"
content = open("JavaScriptCore.xcodeproj/project.pbxproj").read()
for key in filemap:
	content = content.replace(key, filemap[key])
open(proj_file, "wb").write(content)