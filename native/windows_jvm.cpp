#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <jni.h>

namespace fs = std::filesystem;

// todo: at least need
//      server/jvm.dll
//      java.dll
//      jimage.dll
//      jsvml.dll
//      verify.dll
//      lib/modules
constexpr wchar_t JAVA_HOME[] = L"" JAVA_HOME_PATH; // get JAVA_HOME from

typedef jint(JNICALL *CreateJavaVM_t)(JavaVM **pvm, void **env, void *args);
typedef jclass(JNICALL *FindClassFromBootLoader_t)(JNIEnv *env, const char *name);

void loadVmOnWindows() {
    CreateJavaVM_t createJavaVM;
    FindClassFromBootLoader_t findBootClass;

    const auto java_home = std::wstring(JAVA_HOME);
    auto javaDllPath = java_home + L"\\bin\\java.dll";
    bool file_exists = fs::exists(javaDllPath);
    auto size = file_exists ? static_cast<int64_t>(fs::file_size(javaDllPath)) : 0;
    if (!size) {
        std::cerr << "File don't exist: " << javaDllPath.c_str() << "\n";
    }

    auto jvmDllPath = java_home + L"\\bin\\server\\jvm.dll";
    HINSTANCE JVM_DLL = LoadLibraryW(jvmDllPath.c_str());
    if (!JVM_DLL) {
        std::cerr << "Can't load lib from: " << jvmDllPath.c_str() << "\n";
        return;
    }

    createJavaVM = (CreateJavaVM_t)GetProcAddress(JVM_DLL, "JNI_CreateJavaVM");
    findBootClass = (FindClassFromBootLoader_t)GetProcAddress(JVM_DLL, "JVM_FindClassFromBootLoader");
    if (createJavaVM == nullptr || findBootClass == nullptr) {
        std::cout << "GetProcAddress failed: "
            << reinterpret_cast<void*>(createJavaVM) << ", " << reinterpret_cast<void*>(findBootClass) << "\n";
    }

    JNIEnv* jniEnv = nullptr;
    JavaVM* javaVM = nullptr;

    JavaVMOption options[1];
    options[0].optionString = "-Djava.class.path=D:\\work\\graal-native-test\\target\\graal-tutorial-1.1-SNAPSHOT.jar";

    JavaVMInitArgs vmArgs;
    vmArgs.version = JNI_VERSION_1_8;
    vmArgs.options = options;
    vmArgs.nOptions = 1;
    vmArgs.ignoreUnrecognized = JNI_FALSE;

    // Create the JVM
    jint flag = createJavaVM(&javaVM, (void**) &jniEnv, &vmArgs);
    if (flag == JNI_ERR) {
        throw std::runtime_error("Error creating VM. Exiting...n");
    }
}