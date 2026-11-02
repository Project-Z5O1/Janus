

/**
 * FlowDroidAnalysis - Android APK Native 函数静态分析工具
 * 
 * 该工具基于 Soot 和 FlowDroid 框架，用于分析 Android APK 中的 native 函数。
 * 
 * 主要功能：
 * 1. 提取所有 native 函数签名，转换为 JNI 格式
 * 2. 通过调用图分析 native 函数的参数约束（常量、类型、长度依赖、文件路径等）
 * 
 * 输出文件：
 * - signatures_pattern_soot.txt: native 函数签名列表
 * - static_analysis/simple_argument_constraints.txt: 参数约束信息
 * 
 * 使用方法：
 *   java -jar FlowDroidAnalysis.jar [path_to_appfolder] [unused_param]
 */

import soot.Scene;
import soot.SootClass;
import soot.SootMethod;
import soot.Body;
import soot.UnitBox;
import soot.Unit;
import soot.ValueBox;
import soot.Value;
import soot.*;
import soot.jimple.InvokeExpr;
import soot.jimple.internal.*;
import soot.jimple.internal.JAssignStmt.LinkedRValueBox;
import soot.jimple.infoflow.InfoflowConfiguration;
import soot.jimple.infoflow.android.InfoflowAndroidConfiguration;
import soot.jimple.infoflow.android.SetupApplication;
import soot.jimple.toolkits.callgraph.CallGraph;
import soot.jimple.toolkits.callgraph.Edge;
import soot.options.Options;
import soot.jimple.JimpleBody;
import soot.util.HashChain;
import soot.jimple.infoflow.android.config.SootConfigForAndroid;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;

public class FlowDroidAnalysis {
	
	    // ==================== 配置常量 ====================
	    
	    private final static String USER_HOME = System.getProperty("user.home");
	    
	    // Android SDK platforms 路径（用于 FlowDroid 分析）
	    // 服务器路径：/usr/lib/android-sdk/platforms
	    // macOS 路径如下：
	    private static String androidJar = "/Users/syc/Library/Android/sdk/platforms";
	    
	    // 在方法体内向前搜索 native 调用的最大深度
	    private static int bodyNativeSearchDepth = 3;
	    
	    // 在调用图中向后搜索构建调用序列的最大深度
	    private static int callgraphBackwardSearchDepth = 2;
	    
	    // 需要排除的 Android 系统包（减少分析时间，避免分析系统库）
	    public static List<String> androidExclude = Arrays.asList("android.*", "com.google.android.*", "com.android.*", "dalvik.*", "androidx.*", "java.*", "javax.*", "junit.*", "org.apache.*", "sun.*", "org.eclipse.*", "org.json.*", "org.w3c.dom.*", "org.xml.sax.*", "org.xmlpull.*", "kotlin.*");
	    
	    // 实际使用的排除列表
	    public static List<String> exclude = new ArrayList<>();


	    // ==================== 主入口 ====================
	    
	    /**
	     * 程序主入口
	     * 
	     * 执行流程：
	     * 1. 配置 FlowDroid/Soot 分析环境
	     * 2. 构建调用图（CallGraph）
	     * 3. 提取所有 native 函数签名
	     * 4. 分析每个 native 函数的参数约束
	     * 5. 输出结果到文件
	     * 
	     * @param args 命令行参数：[app文件夹路径] [未使用参数]
	     */
	    public static void main(String[] args){
	    	if(args.length != 2){
	    		System.out.println("Usage: [path_to_appfolder]");
	    		System.out.println("retrieves the native function signatures and simple argument contraints");
	    	}
	        String appPath = args[0];
	        String apkPath = appPath + "/base.apk";
	        
	        // ========== 配置 FlowDroid 分析参数 ==========
	        InfoflowAndroidConfiguration config = new InfoflowAndroidConfiguration();
	        config.getAnalysisFileConfig().setTargetAPKFile(apkPath);           // 设置目标 APK 文件
	        config.getAnalysisFileConfig().setAndroidPlatformDir(androidJar);   // 设置 Android SDK 路径
	        config.setCodeEliminationMode(InfoflowConfiguration.CodeEliminationMode.NoCodeElimination);  // 不消除任何代码
	        config.setEnableReflection(false);                                   // 禁用反射分析（提高速度）
	        config.getCallbackConfig().setEnableCallbacks(false);               // 禁用回调分析
	        config.setCallgraphAlgorithm(InfoflowConfiguration.CallgraphAlgorithm.CHA);  // 使用 CHA 算法构建调用图
	        config.setMergeDexFiles(true);                                       // 合并多个 DEX 文件
	        
	        // 创建 FlowDroid 应用分析实例
	        SetupApplication app = new SetupApplication(config);
	        
	        // ========== 配置 Soot 选项 ==========
	        app.setSootConfig(new SootConfigForAndroid() {
                @Override
                public void setSootOptions(Options options, InfoflowConfiguration config) {
                      super.setSootOptions(options, config);  // 调用父类配置
                      
                      // 保留行号信息（用于调试和定位）
                      options.set_keep_line_number(true);
                      options.set_print_tags_in_output(true);
                      
                      // 设置输入源为 APK 格式
                  options.set_src_prec(soot.options.Options.src_prec_apk);
                      
                      // 修复 Android 静态方法问题：https://github.com/soot-oss/soot/issues/1019
                      options.set_wrong_staticness(Options.wrong_staticness_fix);

                      // 添加排除的系统包（减少分析范围，提高速度）
                      exclude.addAll(androidExclude);
                      
                      // 强制解析 JobIntentService（避免某些依赖问题）
                      Scene.v().forceResolve("androidx.core.app.JobIntentService", SootClass.BODIES);
                      options.set_exclude(exclude);

                      // 对于排除的包，不加载方法体（即使调用图构建需要）
                      options.set_no_bodies_for_excluded(true);
                      
                      // 允许幻影引用（未解析的类引用不报错）
                      options.set_allow_phantom_refs(true);
                      
                      // 启用全程序分析模式
                      options.set_whole_program(true);
                }
              });

	        // ========== 构建调用图 ==========
	        // 必须先调用此方法，才能使用 Soot API
	        // 注意：对于大型 APP 可能需要很长时间或不终止
	        app.constructCallgraph();
	        
	        // 提取所有 native 函数
	        List<SootMethod> nativeMethods = findAllNativeFunctions();
	        
	        // ========== 输出 Native 函数签名 ==========
	        if(true) {	        	
		        try{
		        	// 将 native 函数签名写入文件
		        	String nativeMethods_output = "";
		        	for(SootMethod nativeMethod : nativeMethods) {
		        		nativeMethods_output = nativeMethods_output + soot2androlibMethod(nativeMethod) + "\n";
		        	}
		        	String signatures_pattern = appPath + "/signatures_pattern_soot.txt";
		        	FileOutputStream outputStream = new FileOutputStream(signatures_pattern, false);
		        	byte[] strToBytes = nativeMethods_output.getBytes();
		            outputStream.write(strToBytes);
		            outputStream.close();
		        }
		        catch (IOException e){
		            e.printStackTrace();
		            System.exit(-1);
		        }
	        }
	        
	        // ========== 分析并输出参数约束 ==========
	        if(true) {
	        	// 收集所有 native 函数的参数约束信息
	        	String all_constraints = "";
	        	for(SootMethod nativeMethod : nativeMethods) {
	        		// 分析每个 native 函数的参数约束
	        		all_constraints = all_constraints + findArgumentConstraints(nativeMethod);
	        	}
	        	try{
		        	// write output to path
		        	String static_analysis = appPath + "/static_analysis";
		        	Files.createDirectories(Paths.get(static_analysis));
		        	String constraint_output = static_analysis + "/simple_argument_constraints.txt";
		        	FileOutputStream outputStream = new FileOutputStream(constraint_output, false);
		        	byte[] strToBytes = all_constraints.getBytes();
		            outputStream.write(strToBytes);
		            outputStream.close();
		        }
		        catch (IOException e){
		            e.printStackTrace();
		            System.exit(-1);
		        }     	
	        }
	    }
	    
	    // ==================== Soot 配置方法 ====================
	    
	    /**
	     * 应用全程序分析的 Soot 配置选项
	     * 
	     * @param apkFilePath APK 文件路径
	     */
	    public static void applyWholeProgramSootOptions(String apkFilePath) {
            Options.v().set_src_prec(Options.src_prec_apk);  // 设置输入格式为 APK
            //Options.v().set_include_all(true);

            Options.v().set_whole_program(true);
            Options.v().set_allow_phantom_refs(true);
            Options.v().set_time(false);
            //Options.v().set_exclude(exclude);
            //Options.v().set_on_the_fly(true);

            //Options.v().set_exclude(exclusionList);
            Options.v().set_no_bodies_for_excluded(true);
            //Options.v().set_show_exception_dests(false);
            Options.v().set_verbose(false);
    Options.v().set_process_multiple_dex(true);
            //PhaseOptions.v().setPhaseOption("cg", "verbose:true");
            Options.v().set_android_jars(System.getenv("ANDROID_HOME"));
            List<String> processDirs = new ArrayList<String>();
            processDirs.add(apkFilePath);
            Options.v().set_process_dir(processDirs);
            // to mitigate an Android problem: https://github.com/soot-oss/soot/issues/1019
            Options.v().set_wrong_staticness(Options.wrong_staticness_fix);

            Options.v().set_keep_line_number(true);
            Options.v().set_coffi(true);
            //PackManager.v().runPacks();
            Scene.v().loadNecessaryClasses();
    }

	    
	    // ==================== Native 函数查找方法 ====================
	    
	    /**
	     * 在方法体内递归查找 native 调用
	     * 
	     * 遍历方法的 Jimple 中间表示，查找所有 native 函数调用。
	     * 如果遇到普通方法调用，会递归进入该方法继续查找（受 depth 限制）。
	     * 
	     * @param sootMethod 要分析的方法
	     * @param depth 递归搜索深度
	     * @return 找到的 native 方法列表
	     */
	    public static List<SootMethod> findNativeCallsinBody(SootMethod sootMethod, int depth){
	    	List<SootMethod> output = new ArrayList<>();
	    	if(depth == 0) {
	    		return output;
	    	}
	    	if(sootMethod.isJavaLibraryMethod()) {
	    		return output;
	    	}
	    	Body body;
	    	try {
	    		body = sootMethod.retrieveActiveBody();
	    	} catch(java.lang.RuntimeException e) {
	    		// something went wrong, most likely does not have an active body
	    		System.out.println("[-] something went wrong, while looking for active body, skipping...");
	    		return output;
	    	}
	    	
	    	// 遍历方法体中的每个语句单元（Unit）
	    	for(Unit unit : body.getUnits()){
	    		// 处理赋值语句，如：a = nativeFunction()
	    		// 需要从 JAssignStmt.rightBox.value.methodRef 获取方法引用
	    		if(unit instanceof JAssignStmt) {
	    			ValueBox valuebox;
	    			try {
	    				valuebox = ((JAssignStmt) unit).getInvokeExprBox();
	    			} catch(java.lang.RuntimeException e) {
	    				// if the assignstatement does not have an invokeexpression
	    				// just skip this unit
	    				continue;
	    			}
	    			if(valuebox instanceof LinkedRValueBox){
                        Value method2 = valuebox.getValue();
                        if(method2 instanceof JStaticInvokeExpr || method2 instanceof JVirtualInvokeExpr){
                            InvokeExpr meethod3 = (InvokeExpr)method2;
                            SootMethod method = meethod3.getMethod();
                            SootClass declaringClass = method.getDeclaringClass();
                            if(declaringClass.getName().indexOf("java.") != -1 || declaringClass.getName().indexOf("android.") != -1){
                                // we don't care about native functions of the JVM or of android
                                continue;
                            }
                            if(method.isNative()){
                                output.add(method);
                                continue;
                            }
                            output.addAll(findNativeCallsinBody(method, depth-1));
                        }                           
                    }  
	    		}
                if(unit instanceof JInvokeStmt){
                    // it's a function call!
                    for(ValueBox valuebox : unit.getUseBoxes()){
                        // look for invokeexpr to extract the method
                        if(valuebox instanceof InvokeExprBox){
                            Value method2 = valuebox.getValue();
                            if(method2 instanceof JStaticInvokeExpr || method2 instanceof JVirtualInvokeExpr){
                                InvokeExpr meethod3 = (InvokeExpr)method2;
                                SootMethod method = meethod3.getMethod();
                                SootClass declaringClass = method.getDeclaringClass();
                                if(declaringClass.getName().indexOf("java.") != -1 || declaringClass.getName().indexOf("android.") != -1){
                                    // we don't care about native functions of the JVM or of android
                                    continue;
                                }
                                if(method.isNative()){
                                    output.add(method);
                                    continue;
                                }
                                output.addAll(findNativeCallsinBody(method, depth-1));
                            }                           
                        }                        
                    }                  
                }     
	    	}
	    	return output;	    		    
	    } 
	    
	    /**
	     * 查找 APK 中所有的 native 函数
	     * 
	     * 遍历 Soot 场景中的所有类，提取声明为 native 的方法。
	     * 排除 JVM 和 Android 系统的 native 函数。
	     * 
	     * @return native 方法列表
	     */
	    public static List<SootMethod> findAllNativeFunctions() {
	    	List<SootMethod> nativeMethods = new ArrayList<>();
	    	
	    	// 遍历所有已加载的类
	    	for (SootClass sootClass : Scene.v().getClasses()) {
	            // 遍历类中的所有方法
	            for(SootMethod sootMethod : sootClass.getMethods()){
	            	// 检查是否为 native 方法
	            	if(sootMethod.isNative()) {
		            	SootClass declaringClass = sootMethod.getDeclaringClass();
		            	// 排除 JVM 和 Android 系统的 native 函数
	            		if(declaringClass.getName().indexOf("java.") == -1 && declaringClass.getName().indexOf("android.") == -1){
	            			nativeMethods.add(sootMethod);
                    }	            		
	            	}	  
	            }	          
	    	}
	    	return nativeMethods;
	    }	    /**
	     * 从调用图边中提取 native 函数
	     * 
	     * 遍历源方法的方法体，直到到达目标方法，并跟踪调用（受深度限制）。
	     * 
	     * @param edge 调用图中的边
	     * @param depth 搜索深度
	     * @return 找到的 native 方法列表
	     */
	    public static List<SootMethod> ExtractNativeFromEdge(Edge edge, int depth){
	    	SootMethod srcMethod = edge.src();  // 调用者方法
	    	SootMethod dstMethod = edge.tgt();
	    	List<SootMethod> output = new ArrayList<>();
	    	if(srcMethod.isJavaLibraryMethod()) {
	    		return output;
	    	}
	    	Body body;
	    	try {
	    		body = srcMethod.retrieveActiveBody();
	    	} catch(java.lang.RuntimeException e) {
	    		// something went wrong, most likely does not have an active body
	    		System.out.println("[-] something went wrong, while looking for active body, skipping...");
	    		return output;
	    	}
	    	
	    	return output;
	    }
	    
	    // ==================== 参数约束分析方法 ====================
	    
	    /**
	     * 分析 native 函数的参数约束
	     * 
	     * 通过自底向上的分析，尝试获取 native 函数输入参数的约束信息。
	     * 
	     * 分析内容包括：
	     * 1. 常量参数：直接传入的常量值
	     * 2. 参数类型：参数的 Java 类型
	     * 3. 长度依赖：某个参数是另一个参数的长度
	     * 4. 文件路径：参数是否为文件路径
	     * 
	     * @param method 要分析的 native 方法
	     * @return 约束信息字符串
	     */
	    public static String findArgumentConstraints(SootMethod method) {
	    	String output = "";
	    	
	    	// 查找所有调用此方法的边（即找到所有调用点）
	    	List<Edge> incomingEdges = findUsage(method);
	    	
	    	// 遍历每个调用点，分析参数
	    	for(Edge edge : incomingEdges) {
	    		Unit unit = edge.srcUnit();  // 获取调用点的语句
	    		
	    		// 处理普通调用语句：nativeFunc(arg1, arg2)
	    		if(unit instanceof JInvokeStmt) {
	    			JInvokeStmt jstmt = (JInvokeStmt)unit;
	    			InvokeExpr invokeExpr = jstmt.getInvokeExpr();
	    			List<Value> args = invokeExpr.getArgs();  // 获取所有参数
	    			int index = 0;
	    			
	    			// 遍历每个参数进行分析
	    			for(Value argument : args) {
	    				// 情况1：参数是常量（非局部变量）
	    				if(argument instanceof Immediate && !(argument instanceof JimpleLocal)) {
	    					System.out.println("[:)] found constant for native function at index : " + index + method.toString() + argument.toString());
	    					output = output + "native_function_constant: " +method.toString() + ":" + index + ":" + argument.toString() + "\n";
	    				}
	    				if(argument instanceof Immediate && argument instanceof JimpleLocal) {
	    					Body body;
	    					try {
	    			    		body = edge.getSrc().method().retrieveActiveBody();
	    			    		// search through the active body of the calling edge and look for some constraints
		    					String out = findArgConstraints(method, body, argument, index, args, unit);
		    					if(!(out.equals("NONE"))) {
		    						output = output + out + "\n";
		    					}
	    			    	} catch(java.lang.RuntimeException e) {
	    			    		// something went wrong, most likely does not have an active body
	    			    		System.out.println("[-] something went wrong, while looking for active body, skipping...");
	    			    	}
	    				}
	    				if(argument instanceof Immediate && argument instanceof JimpleLocal) {
	    					// try to figure out the type of this argument
	    					Type type = argument.getType();
	    					System.out.println("[:)] class found for native function at index : " + index + method.toString() + type.toString());
	    					output = output + "native_function_argumentclass: " +method.toString() + ":" + index + ":" + type.toString() + "\n";

	    				}
	    				index += 1;
	    			}
	    			continue;
	    		}
	    		if(unit instanceof JAssignStmt) {
	    			ValueBox valuebox;
	    			try {
	    				valuebox = ((JAssignStmt) unit).getInvokeExprBox();
	    			} catch(java.lang.RuntimeException e) {
	    				// if the assignstatement does not have an invokeexpression
	    				// just skip this unit
	    				continue;
	    			}
	    			if(valuebox instanceof LinkedRValueBox){
                        Value method2 = valuebox.getValue();
                        if(method2 instanceof JStaticInvokeExpr || method2 instanceof JVirtualInvokeExpr){
                            InvokeExpr invokeExpr = (InvokeExpr)method2;
                            List<Value> args = invokeExpr.getArgs();
        	    			int index = 0;
        	    			for(Value argument : args) {
        	    				if(argument instanceof Immediate && !(argument instanceof JimpleLocal)) {
        	    					System.out.println("[:)] found constant for native function at index : " + index + method.toString() + argument.toString());
        	    					output = output + "native_function_constant: " +method.toString() + ":" + index + ":" + argument.toString() + "\n";
        	    				}
        	    				if(argument instanceof Immediate && argument instanceof JimpleLocal) {
        	    					Body body;
        	    					try {
        	    			    		body = edge.getSrc().method().retrieveActiveBody();
        	    			    		// search through the active body of the calling edge and look for some constraints
        		    					String out = findArgConstraints(method, body, argument, index, args, unit);
        		    					if(!(out.equals("NONE"))) {
        		    						output = output + out + "\n";
        		    					}
        	    			    	} catch(java.lang.RuntimeException e) {
        	    			    		// something went wrong, most likely does not have an active body
        	    			    		System.out.println("[-] something went wrong, while looking for active body, skipping...");
        	    			    	}
        	    				}
        	    				if(argument instanceof Immediate && argument instanceof JimpleLocal) {
        	    					// try to figure out the type of this argument
        	    					Type type = argument.getType();
        	    					System.out.println("[:)] class found for native function at index : " + index + method.toString() + type.toString());
        	    					output = output + "native_function_argumentclass: " +method.toString() + ":" + index + ":" + type.toString() + "\n";

        	    				}
        	    				index += 1;
        	    			}
        	    			continue;
                        }
	    			}
	    		}
	    	}
	    	
	    	return output;
	    }
	    
	    /**
	     * 分析单个参数的约束条件
	     * 
	     * 针对不同类型的参数进行特定分析：
	     * - int 类型：检查是否为另一个参数的长度
	     * - String 类型：检查是否为文件路径
	     * 
	     * @param method native 方法
	     * @param body 调用者方法的方法体
	     * @param argument 要分析的参数
	     * @param index 参数索引
	     * @param args 所有参数列表
	     * @param original_unit 原始调用语句
	     * @return 约束信息字符串，无约束返回 "NONE"
	     */
	    public static String findArgConstraints(SootMethod method, Body body, Value argument, int index, List<Value> args, Unit original_unit) {
	    	
	    	// ===== 分析 int 类型参数 =====
	    	if(argument.getType().toString().equals("int")) {
	    		// 检查是否为另一个参数的长度（如：len = array.length）
	    		String arg_name = argument.toString();
	    		
	    		// 查找该参数在调用前的赋值语句
	    		Unit unit = get_unit_for_var_before_unit(body, original_unit, arg_name);
	    		if(unit != null) {
	    			// 找到了赋值语句，分析右侧表达式
    				if(unit instanceof JAssignStmt) {
    					JAssignStmt assignStmt = (JAssignStmt)unit;
    					ValueBox rside = assignStmt.getRightOpBox();
    					Value rside2 = rside.getValue();
    					
    					// 检查是否为长度表达式：arg = lengthof array
    					if(rside2 instanceof JLengthExpr) {
    						JLengthExpr rside3 = (JLengthExpr)rside2;
    						String lengthof_arg = rside3.getOpBox().getValue().toString();
    						
    						// 检查 lengthof 的参数是否也是 native 函数的参数
    						int arg_index = find_in_args(args, lengthof_arg);
    						if(arg_index != -1) {
    							System.out.println("[!] length dependency" + method.toString() + "found a length dependency between: args[" + index + "] = len(args["+ arg_index+"]");
    							return "[!] length dependency" + method.toString() + "found a length dependency between: args[" + index + "] = len(args["+ arg_index+"]";
    						}
    					}
    				}
	    		} 		
	    	}
	    	
	    	// ===== 分析 String 类型参数 =====
	    	if(argument.getType().toString().equals("java.lang.String")) {
	    		// 检查是否为文件路径
	    		String arg_name = argument.toString();
	    		Unit unit = get_unit_for_var_before_unit(body, original_unit, arg_name);
	    		if(unit != null) {
	    			if(unit instanceof JAssignStmt) {
    					JAssignStmt assignStmt = (JAssignStmt)unit;
    					ValueBox rside = assignStmt.getRightOpBox();
    					Value rside2 = rside.getValue();
    					if(rside2 instanceof JVirtualInvokeExpr) {
    						// arg = virtualinvoke ...
    						JVirtualInvokeExpr rside3 = (JVirtualInvokeExpr)rside2;
    						SootMethod rside_meth = rside3.getMethod();
    						String rcls = rside_meth.getDeclaringClass().toString();
    						String rfname = rside_meth.getName().toString();
    						if(is_filepath(rcls, rfname)) {
    							System.out.println("[!] filepath constraint" + method.toString() + "found a filepath constraint for arg at index: " + index);
    							return "[!] filepath constraint" + method.toString() + "found a filepath constraint for arg at index: " + index;
    						}
    					}
    				}
	    		}
	    	}
	    	return "NONE";
	    }
	    
	    
	    // ==================== 辅助方法 ====================
	    
	    /**
	     * 在目标语句之前查找变量的最近赋值语句
	     * 
	     * @param body 方法体
	     * @param targetUnit 目标语句（在此之前搜索）
	     * @param arg_name 变量名
	     * @return 赋值语句，未找到返回 null
	     */
	    public static Unit get_unit_for_var_before_unit(Body body, Unit targetUnit, String arg_name) {
	    	String assign_arg = arg_name + " =";  // 构建赋值模式
	    	Unit mostrecentUnit = null;
	    	for(Unit unit : body.getUnits()){
	    		if(unit.toString().equals(targetUnit.toString())) {
	    			break;
	    		}
	    		if(unit.toString().contains(assign_arg)) {
	    			mostrecentUnit = unit;
	    		}
	    	}
	    	return mostrecentUnit;
	    }
	    
	    /**
	     * 判断方法调用是否返回文件路径
	     * 
	     * 检查 java.io.File 类的路径获取方法。
	     * 
	     * @param cls 类名
	     * @param method 方法名
	     * @return 是文件路径方法返回 true
	     */
	    public static boolean is_filepath(String cls, String method) {
	    	if(cls.equals("java.io.File")) {
	    		// 检查是否为路径获取方法
	    		if(method.equals("getPath") || method.equals("getAbsolutePath") || method.equals("getCanonicalPath") 
	    				|| method.equals("getAbsoluteFile")) {
	    			return true;
	    		}
	    	}
	    	return false;
	    }
	    
	    /**
	     * 在参数列表中查找指定参数
	     * 
	     * @param args 参数列表
	     * @param search_arg 要查找的参数名
	     * @return 参数索引，未找到返回 -1
	     */
	    public static int find_in_args(List<Value> args, String search_arg){
	    	int i = 0;
	    	for(Value arg : args){
	    		if(search_arg == arg.toString()) return i;
	    		i ++;
	    	}
	    	return -1;
	    }
	    
	    // ==================== 调用图分析方法 ====================
	    
	    /**
	     * 递归构建调用序列
	     * 
	     * 沿调用图向上追溯，构建到达目标方法的所有可能调用序列。
	     * 
	     * @param method 目标方法
	     * @param depth 向上追溯的深度
	     * @return 调用序列列表（每个序列是一个边列表）
	     */
	    public static List<List<Edge>> usageSequenceRecurse(SootMethod method, int depth){
	    	List<List<Edge>> allSequences = new ArrayList<>();
	    	
	    	// 达到深度限制，返回空序列
	    	if(depth == 0) {
	    		allSequences.add(new ArrayList<Edge>());
	    		return allSequences;
	    	}
	    	
	    	// 获取所有调用此方法的边
	    	List<Edge> incomingEdges = findUsage(method);
	    	
	    	// 递归处理每个调用者
	    	for(Edge inc : incomingEdges) {
	    		SootMethod parent = inc.src();  // 获取调用者方法
	    		List<List<Edge>> sequenceList = usageSequenceRecurse(parent, depth-1);
	    		
	    		// 将当前边添加到每个序列的开头
	    		for(List<Edge> callsequence : sequenceList) {
	    			callsequence.add(0, inc);
	    		}
	    		allSequences.addAll(sequenceList);
	    	}
	    	return allSequences;
	    }
	    
	    /**
	     * 查找方法的所有调用点
	     * 
	     * 在调用图中查找所有指向该方法的边。
	     * 
	     * @param method 目标方法
	     * @return 入边列表（即调用此方法的所有边）
	     */
	    public static List<Edge> findUsage(SootMethod method){
	    	List<Edge> incomingEdges = new ArrayList<Edge>();
	    	
	    	// 从 Soot 场景获取调用图
	    	CallGraph callgraph = Scene.v().getCallGraph();
	    	
	    	// 获取所有指向该方法的边
	    	Iterator<Edge> it = callgraph.edgesInto(method);
	    	while(it.hasNext()) {
	    		Edge edge = it.next();
	    		incomingEdges.add(edge);
	    	}
	    	return incomingEdges;
	    }
	    
	    // ==================== 签名转换方法 ====================
	  	    
	    /**
	     * 将 Soot 方法转换为 JNI 格式签名
	     * 
	     * 将 Soot 的方法表示转换为 JNI 命名约定的格式。
	     * 输出格式与 jadx + qdox 工具相同。
	     * 
	     * 转换规则：
	     * - 前缀：Java_
	     * - 类名：将 "." 替换为 "_"，"_" 替换为 "_1"
	     * - 方法名："_" 替换为 "_1"
	     * 
	     * 示例：com.example.Test.nativeFunc -> Java_com_example_Test_nativeFunc
	     * 
	     * @param method Soot 方法对象
	     * @return JNI 格式的方法签名字符串
	     */
	    public static String soot2androlibMethod(SootMethod method) {
	    	String prefix = "Java";
	    	
	    	// 处理类名：先转义下划线，再将点替换为下划线
	    	String className = method.getDeclaringClass().getName();
	    	className = className.replace("_", "_1");
	    	className = className.replace(".", "_");
	    	
	    	// 处理方法名：转义下划线
	    	String functionName = method.getName();
	    	functionName = functionName.replace("_", "_1");
	    	
	    	// 获取返回类型（仅基本类型名）
	    	String returnType = getBaseType(method.getReturnType().toString());
	    	
	    	// 构建参数列表字符串
	    	String arguments = "";
	    	for(Type param : method.getParameterTypes()) {
	    		arguments = arguments + getBaseType(param.toString()) + ",";
	    	}	
	    	
	    	// 格式：Java_类名_方法名 返回类型:参数1,参数2,...
	    	return prefix + "_"  + className + "_" + functionName + " " + returnType + ":" + arguments;
	    }
	    
	    /**
	     * 获取类型的基本名称
	     * 
	     * 去掉包名前缀，只保留类型名。
	     * 例如：java.lang.String -> String
	     * 
	     * @param type 完整类型名
	     * @return 基本类型名
	     */
	    public static String getBaseType(String type) {
	    	return type.substring(type.lastIndexOf('.') + 1);
	    }
}

