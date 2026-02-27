#include <jni.h>
#include <android/log.h>
#include <string>

#define TAG "ModdedByAlebalasov"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// Глобальные ссылки на классы (чтобы не искать каждый раз)
static jclass class_Settings = nullptr;
static jclass class_Intent = nullptr;
static jclass class_Uri = nullptr;
static jclass class_Context = nullptr;
static jclass class_WindowManager = nullptr;
static jclass class_LayoutParams = nullptr;
static jclass class_TextView = nullptr;
static jclass class_Handler = nullptr;

extern "C" {

/**
 * Инициализация глобальных ссылок
 */
JNIEXPORT void JNICALL
Java_com_mod_OverlayMod_init(JNIEnv *env, jclass clazz) {
    LOGD("Initializing global references");
    
    if (class_Settings == nullptr) {
        jclass local = env->FindClass("android/provider/Settings");
        class_Settings = (jclass)env->NewGlobalRef(local);
    }
    
    if (class_Intent == nullptr) {
        jclass local = env->FindClass("android/content/Intent");
        class_Intent = (jclass)env->NewGlobalRef(local);
    }
    
    if (class_Uri == nullptr) {
        jclass local = env->FindClass("android/net/Uri");
        class_Uri = (jclass)env->NewGlobalRef(local);
    }
    
    if (class_Context == nullptr) {
        jclass local = env->FindClass("android/content/Context");
        class_Context = (jclass)env->NewGlobalRef(local);
    }
    
    if (class_WindowManager == nullptr) {
        jclass local = env->FindClass("android/view/WindowManager");
        class_WindowManager = (jclass)env->NewGlobalRef(local);
    }
    
    if (class_LayoutParams == nullptr) {
        jclass local = env->FindClass("android/view/WindowManager$LayoutParams");
        class_LayoutParams = (jclass)env->NewGlobalRef(local);
    }
    
    if (class_TextView == nullptr) {
        jclass local = env->FindClass("android/widget/TextView");
        class_TextView = (jclass)env->NewGlobalRef(local);
    }
    
    if (class_Handler == nullptr) {
        jclass local = env->FindClass("android/os/Handler");
        class_Handler = (jclass)env->NewGlobalRef(local);
    }
    
    LOGD("Global references initialized");
}

/**
 * Проверка наличия разрешения SYSTEM_ALERT_WINDOW
 */
JNIEXPORT jboolean JNICALL
Java_com_mod_OverlayMod_checkOverlayPermission(JNIEnv *env, jclass clazz, jobject context) {
    LOGD("Checking overlay permission");
    
    if (class_Settings == nullptr) {
        Java_com_mod_OverlayMod_init(env, clazz);
    }
    
    // Получаем метод canDrawOverlays
    jmethodID canDrawMethod = env->GetStaticMethodID(
        class_Settings,
        "canDrawOverlays",
        "(Landroid/content/Context;)Z"
    );
    
    if (canDrawMethod == nullptr) {
        LOGD("Failed to find canDrawOverlays method");
        return JNI_FALSE;
    }
    
    // Вызываем проверку
    jboolean result = env->CallStaticBooleanMethod(class_Settings, canDrawMethod, context);
    LOGD("Permission check result: %d", result);
    
    return result;
}

/**
 * Запрос разрешения (открывает системные настройки)
 */
JNIEXPORT void JNICALL
Java_com_mod_OverlayMod_requestOverlayPermission(JNIEnv *env, jclass clazz, jobject context) {
    LOGD("Requesting overlay permission");
    
    if (class_Settings == nullptr || class_Intent == nullptr || class_Uri == nullptr) {
        Java_com_mod_OverlayMod_init(env, clazz);
    }
    
    // Получаем константу ACTION_MANAGE_OVERLAY_PERMISSION
    jfieldID actionField = env->GetStaticFieldID(
        class_Settings,
        "ACTION_MANAGE_OVERLAY_PERMISSION",
        "Ljava/lang/String;"
    );
    
    if (actionField == nullptr) {
        LOGD("Failed to find ACTION_MANAGE_OVERLAY_PERMISSION");
        return;
    }
    
    jstring actionString = (jstring)env->GetStaticObjectField(class_Settings, actionField);
    
    // Получаем конструктор Intent(String)
    jmethodID intentConstructor = env->GetMethodID(class_Intent, "<init>", "(Ljava/lang/String;)V");
    
    // Создаём Intent с action
    jobject intent = env->NewObject(class_Intent, intentConstructor, actionString);
    
    // Получаем package name
    jmethodID getPackageNameMethod = env->GetMethodID(class_Context, "getPackageName", "()Ljava/lang/String;");
    jstring packageName = (jstring)env->CallObjectMethod(context, getPackageNameMethod);
    
    // Создаём строку "package:имя.пакета"
    jstring packagePrefix = env->NewStringUTF("package:");
    
    // Конкатенация строк
    jclass stringClass = env->FindClass("java/lang/String");
    jmethodID concatMethod = env->GetMethodID(stringClass, "concat", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject uriString = env->CallObjectMethod(packagePrefix, concatMethod, packageName);
    
    // Парсим URI
    jmethodID parseMethod = env->GetStaticMethodID(class_Uri, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
    jobject uri = env->CallStaticObjectMethod(class_Uri, parseMethod, uriString);
    
    // Устанавливаем data в intent
    jmethodID setDataMethod = env->GetMethodID(class_Intent, "setData", "(Landroid/net/Uri;)Landroid/content/Intent;");
    env->CallObjectMethod(intent, setDataMethod, uri);
    
    // Добавляем флаг NEW_TASK
    jfieldID flagField = env->GetStaticFieldID(class_Intent, "FLAG_ACTIVITY_NEW_TASK", "I");
    jint flag = env->GetStaticIntField(class_Intent, flagField);
    jmethodID addFlagsMethod = env->GetMethodID(class_Intent, "addFlags", "(I)Landroid/content/Intent;");
    env->CallObjectMethod(intent, addFlagsMethod, flag);
    
    // Запускаем activity
    jmethodID startActivityMethod = env->GetMethodID(class_Context, "startActivity", "(Landroid/content/Intent;)V");
    env->CallVoidMethod(context, startActivityMethod, intent);
    
    LOGD("Permission request intent sent");
}

/**
 * Показать оверлейное окно
 */
JNIEXPORT void JNICALL
Java_com_mod_OverlayMod_showModOverlay(JNIEnv *env, jclass clazz, jobject context) {
    LOGD("showModOverlay called from JNI!");
    
    // Проверяем разрешение
    if (Java_com_mod_OverlayMod_checkOverlayPermission(env, clazz, context) == JNI_FALSE) {
        LOGD("Cannot draw overlays - permission not granted");
        return;
    }
    
    if (class_WindowManager == nullptr || class_TextView == nullptr || class_LayoutParams == nullptr) {
        Java_com_mod_OverlayMod_init(env, clazz);
    }
    
    // 1. Получаем WindowManager
    jmethodID getSystemServiceMethod = env->GetMethodID(
        class_Context,
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;"
    );
    
    jstring windowServiceString = env->NewStringUTF("window");
    jobject windowManager = env->CallObjectMethod(
        context,
        getSystemServiceMethod,
        windowServiceString
    );
    
    if (windowManager == nullptr) {
        LOGD("Failed to get WindowManager");
        return;
    }
    
    // 2. Создаём TextView
    jmethodID textViewConstructor = env->GetMethodID(class_TextView, "<init>", "(Landroid/content/Context;)V");
    jobject textView = env->NewObject(class_TextView, textViewConstructor, context);
    
    // Устанавливаем текст
    jmethodID setTextMethod = env->GetMethodID(class_TextView, "setText", "(Ljava/lang/CharSequence;)V");
    jstring message = env->NewStringUTF("Modded by alebalasov");
    env->CallVoidMethod(textView, setTextMethod, message);
    
    // Устанавливаем цвет фона (полупрозрачный чёрный)
    jmethodID setBgColorMethod = env->GetMethodID(class_TextView, "setBackgroundColor", "(I)V");
    env->CallVoidMethod(textView, setBgColorMethod, (jint)0xCC333333);
    
    // Устанавливаем цвет текста (белый)
    jmethodID setTextColorMethod = env->GetMethodID(class_TextView, "setTextColor", "(I)V");
    env->CallVoidMethod(textView, setTextColorMethod, (jint)0xFFFFFFFF);
    
    // Устанавливаем размер текста (16sp)
    jmethodID setTextSizeMethod = env->GetMethodID(class_TextView, "setTextSize", "(F)V");
    env->CallVoidMethod(textView, setTextSizeMethod, (jfloat)16.0);
    
    // Устанавливаем гравитацию CENTER
    jmethodID setGravityMethod = env->GetMethodID(class_TextView, "setGravity", "(I)V");
    env->CallVoidMethod(textView, setGravityMethod, (jint)0x11); // Gravity.CENTER = 0x11
    
    // 3. Создаём LayoutParams
    // Получаем константы
    jfieldID typeField = env->GetStaticFieldID(
        class_LayoutParams,
        "TYPE_APPLICATION_OVERLAY",
        "I"
    );
    jint type = env->GetStaticIntField(class_LayoutParams, typeField);
    
    jfieldID flagsField = env->GetStaticFieldID(
        class_LayoutParams,
        "FLAG_NOT_FOCUSABLE",
        "I"
    );
    jint notFocusableFlag = env->GetStaticIntField(class_LayoutParams, flagsField);
    
    jfieldID watchOutsideField = env->GetStaticFieldID(
        class_LayoutParams,
        "FLAG_WATCH_OUTSIDE_TOUCH",
        "I"
    );
    jint watchOutsideFlag = env->GetStaticIntField(class_LayoutParams, watchOutsideField);
    
    // Получаем конструктор
    jmethodID lpConstructor = env->GetMethodID(
        class_LayoutParams,
        "<init>",
        "(IIIII)V"
    );
    
    jint width = 600;
    jint height = 200;
    jint format = -3; // PixelFormat.TRANSLUCENT
    
    jobject params = env->NewObject(
        class_LayoutParams,
        lpConstructor,
        width,
        height,
        type,
        notFocusableFlag | watchOutsideFlag,
        format
    );
    
    // Устанавливаем гравитацию TOP | CENTER_HORIZONTAL
    jfieldID gravityField = env->GetFieldID(class_LayoutParams, "gravity", "I");
    env->SetIntField(params, gravityField, (jint)0x31); // Gravity.TOP | Gravity.CENTER_HORIZONTAL
    
    // Устанавливаем отступ сверху
    jfieldID yField = env->GetFieldID(class_LayoutParams, "y", "I");
    env->SetIntField(params, yField, 100);
    
    // 4. Добавляем view
    jclass wmClass = env->GetObjectClass(windowManager);
    jmethodID addViewMethod = env->GetMethodID(
        wmClass,
        "addView",
        "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V"
    );
    
    env->CallVoidMethod(windowManager, addViewMethod, textView, params);
    
    LOGD("Overlay view added successfully");
    
    // 5. Сохраняем для авто-закрытия
    // Для простоты создадим Java-объект с Runnable
    
    // Получаем класс OverlayMod для вызова Java-метода
    jclass overlayModClass = env->FindClass("com/mod/OverlayMod");
    jmethodID scheduleAutoCloseMethod = env->GetStaticMethodID(
        overlayModClass,
        "scheduleAutoClose",
        "(Landroid/content/Context;Landroid/view/View;Landroid/view/WindowManager;)V"
    );
    
    if (scheduleAutoCloseMethod != nullptr) {
        env->CallStaticVoidMethod(
            overlayModClass,
            scheduleAutoCloseMethod,
            context,
            textView,
            windowManager
        );
    }
}

} // extern "C"
