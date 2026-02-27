#include <jni.h>
#include <android/log.h>
#include <string>

#define TAG "ModdedByAlebalasov"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

extern "C" {

/**
 * Проверка наличия разрешения SYSTEM_ALERT_WINDOW
 */
JNIEXPORT jboolean JNICALL
Java_com_mod_OverlayMod_checkOverlayPermission(JNIEnv *env, jobject thiz, jobject context) {
    LOGD("Checking overlay permission");

    // Получаем класс Settings
    jclass settingsClass = env->FindClass("android/provider/Settings");
    if (settingsClass == nullptr) {
        LOGD("Failed to find Settings class");
        return JNI_FALSE;
    }

    // Получаем метод canDrawOverlays
    jmethodID canDrawMethod = env->GetStaticMethodID(
        settingsClass,
        "canDrawOverlays",
        "(Landroid/content/Context;)Z"
    );

    if (canDrawMethod == nullptr) {
        LOGD("Failed to find canDrawOverlays method");
        return JNI_FALSE;
    }

    // Вызываем проверку
    jboolean result = env->CallStaticBooleanMethod(settingsClass, canDrawMethod, context);
    LOGD("Permission check result: %d", result);

    return result;
}

/**
 * Запрос разрешения (открывает системные настройки)
 */
JNIEXPORT void JNICALL
Java_com_mod_OverlayMod_requestOverlayPermission(JNIEnv *env, jobject thiz, jobject context) {
    LOGD("Requesting overlay permission");

    // Получаем класс Intent
    jclass intentClass = env->FindClass("android/content/Intent");
    if (intentClass == nullptr) {
        LOGD("Failed to find Intent class");
        return;
    }

    // Получаем конструктор Intent
    jmethodID intentConstructor = env->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;)V");
    if (intentConstructor == nullptr) {
        LOGD("Failed to find Intent constructor");
        return;
    }

    // Получаем константу ACTION_MANAGE_OVERLAY_PERMISSION
    jfieldID actionField = env->GetStaticFieldID(
        settingsClass,
        "ACTION_MANAGE_OVERLAY_PERMISSION",
        "Ljava/lang/String;"
    );

    if (actionField == nullptr) {
        LOGD("Failed to find ACTION_MANAGE_OVERLAY_PERMISSION");
        return;
    }

    jstring actionString = (jstring)env->GetStaticObjectField(settingsClass, actionField);

    // Создаём Intent с этим action
    jobject intent = env->NewObject(intentClass, intentConstructor, actionString);

    // Добавляем data URI с пакетом приложения
    jclass uriClass = env->FindClass("android/net/Uri");
    if (uriClass != nullptr) {
        jmethodID parseMethod = env->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
        if (parseMethod != nullptr) {
            // Получаем package name
            jclass contextClass = env->GetObjectClass(context);
            jmethodID getPackageNameMethod = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
            jstring packageName = (jstring)env->CallObjectMethod(context, getPackageNameMethod);

            // Создаём строку "package:имя.пакета"
            jstring packagePrefix = env->NewStringUTF("package:");
            jclass stringClass = env->FindClass("java/lang/String");
            jmethodID concatMethod = env->GetMethodID(stringClass, "concat", "(Ljava/lang/String;)Ljava/lang/String;");
            jobject uriString = env->CallObjectMethod(packagePrefix, concatMethod, packageName);

            // Парсим URI
            jobject uri = env->CallStaticObjectMethod(uriClass, parseMethod, uriString);

            // Устанавливаем data в intent
            jmethodID setDataMethod = env->GetMethodID(intentClass, "setData", "(Landroid/net/Uri;)Landroid/content/Intent;");
            env->CallObjectMethod(intent, setDataMethod, uri);
        }
    }

    // Добавляем флаг NEW_TASK
    jclass contextClass = env->GetObjectClass(context);
    jmethodID startActivityMethod = env->GetMethodID(
        contextClass,
        "startActivity",
        "(Landroid/content/Intent;)V"
    );

    if (startActivityMethod != nullptr) {
        env->CallVoidMethod(context, startActivityMethod, intent);
        LOGD("Permission request intent sent");
    }
}

/**
 * Показать оверлейное окно
 */
JNIEXPORT void JNICALL
Java_com_mod_OverlayMod_showModOverlay(JNIEnv *env, jobject thiz, jobject context) {
    LOGD("showModOverlay called from JNI!");

    // Проверяем разрешение
    jclass settingsClass = env->FindClass("android/provider/Settings");
    jmethodID canDrawMethod = env->GetStaticMethodID(
        settingsClass,
        "canDrawOverlays",
        "(Landroid/content/Context;)Z"
    );

    jboolean canDraw = env->CallStaticBooleanMethod(settingsClass, canDrawMethod, context);

    if (!canDraw) {
        LOGD("Cannot draw overlays - permission not granted");
        return;
    }

    // 1. Создаём WindowManager
    jclass contextClass = env->GetObjectClass(context);
    jmethodID getSystemServiceMethod = env->GetMethodID(
        contextClass,
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

    // 2. Получаем LayoutInflater
    jclass layoutInflaterClass = env->FindClass("android/view/LayoutInflater");
    jmethodID fromMethod = env->GetStaticMethodID(
        layoutInflaterClass,
        "from",
        "(Landroid/content/Context;)Landroid/view/LayoutInflater;"
    );

    jobject inflater = env->CallStaticObjectMethod(layoutInflaterClass, fromMethod, context);

    // 3. Создаём простой TextView (без XML)
    jclass textViewClass = env->FindClass("android/widget/TextView");
    jmethodID textViewConstructor = env->GetMethodID(textViewClass, "<init>", "(Landroid/content/Context;)V");
    jobject textView = env->NewObject(textViewClass, textViewConstructor, context);

    // Устанавливаем текст
    jmethodID setTextMethod = env->GetMethodID(textViewClass, "setText", "(Ljava/lang/CharSequence;)V");
    jstring message = env->NewStringUTF("Modded by alebalasov");
    env->CallVoidMethod(textView, setTextMethod, message);

    // Устанавливаем цвет фона (полупрозрачный чёрный)
    jmethodID setBgColorMethod = env->GetMethodID(textViewClass, "setBackgroundColor", "(I)V");
    env->CallVoidMethod(textView, setBgColorMethod, (jint)0xCC333333); // ARGB формат

    // Устанавливаем цвет текста (белый)
    jmethodID setTextColorMethod = env->GetMethodID(textViewClass, "setTextColor", "(I)V");
    env->CallVoidMethod(textView, setTextColorMethod, (jint)0xFFFFFFFF);

    // Устанавливаем размер текста (16sp)
    jmethodID setTextSizeMethod = env->GetMethodID(textViewClass, "setTextSize", "(F)V");
    env->CallVoidMethod(textView, setTextSizeMethod, (jfloat)16.0);

    // Устанавливаем отступы (16dp)
    jmethodID setPaddingMethod = env->GetMethodID(textViewClass, "setPadding", "(IIII)V");
    jfloat density = 2.0; // пример для плотности экрана, в реальности нужно вычислять
    jint padding = (jint)(16 * density);
    env->CallVoidMethod(textView, setPaddingMethod, padding, padding, padding, padding);

    // Устанавливаем гравитацию CENTER
    jmethodID setGravityMethod = env->GetMethodID(textViewClass, "setGravity", "(I)V");
    env->CallVoidMethod(textView, setGravityMethod, (jint)0x11); // Gravity.CENTER = 0x11

    // 4. Создаём LayoutParams для оверлея
    jclass layoutParamsClass = env->FindClass("android/view/WindowManager$LayoutParams");

    // Получаем константы
    jfieldID typeField = env->GetStaticFieldID(
        layoutParamsClass,
        "TYPE_APPLICATION_OVERLAY",
        "I"
    );
    jint type = env->GetStaticIntField(layoutParamsClass, typeField);

    jfieldID flagsField = env->GetStaticFieldID(
        layoutParamsClass,
        "FLAG_NOT_FOCUSABLE",
        "I"
    );
    jint notFocusableFlag = env->GetStaticIntField(layoutParamsClass, flagsField);

    jfieldID watchOutsideField = env->GetStaticFieldID(
        layoutParamsClass,
        "FLAG_WATCH_OUTSIDE_TOUCH",
        "I"
    );
    jint watchOutsideFlag = env->GetStaticIntField(layoutParamsClass, watchOutsideField);

    // Создаём объект LayoutParams
    jmethodID lpConstructor = env->GetMethodID(
        layoutParamsClass,
        "<init>",
        "(IIIII)V"
    );

    jint width = 600; // px
    jint height = 200; // px
    jint format = -3; // PixelFormat.TRANSLUCENT

    jobject params = env->NewObject(
        layoutParamsClass,
        lpConstructor,
        width,
        height,
        type,
        notFocusableFlag | watchOutsideFlag,
        format
    );

    // Устанавливаем гравитацию TOP | CENTER_HORIZONTAL
    jfieldID gravityField = env->GetFieldID(layoutParamsClass, "gravity", "I");
    env->SetIntField(params, gravityField, (jint)0x31); // Gravity.TOP | Gravity.CENTER_HORIZONTAL = 0x30 | 0x01 = 0x31

    // Устанавливаем отступ сверху (100px)
    jfieldID yField = env->GetFieldID(layoutParamsClass, "y", "I");
    env->SetIntField(params, yField, 100);

    // 5. Добавляем view в WindowManager
    jclass wmClass = env->GetObjectClass(windowManager);
    jmethodID addViewMethod = env->GetMethodID(
        wmClass,
        "addView",
        "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V"
    );

    env->CallVoidMethod(windowManager, addViewMethod, textView, params);

    LOGD("Overlay view added successfully");

    // 6. Устанавливаем автоматическое закрытие через 5 секунд
    jclass handlerClass = env->FindClass("android/os/Handler");
    jmethodID handlerConstructor = env->GetMethodID(handlerClass, "<init>", "()V");
    jobject handler = env->NewObject(handlerClass, handlerConstructor);

    jclass runnableClass = env->FindClass("java/lang/Runnable");
    jclass runnableImplClass = env->FindClass("com/mod/AutoCloseRunnable");

    if (runnableImplClass == nullptr) {
        // Если нет готового класса, создаём анонимный Runnable через JNI сложно
        // Поэтому лучше обрабатывать закрытие из Java-кода
        LOGD("Auto-close would be here (implement in Java for simplicity)");
    }

    // 7. Настраиваем кнопку закрытия (просто по тапу на окно)
    jclass viewClass = env->FindClass("android/view/View");
    jmethodID setOnClickListenerMethod = env->GetMethodID(
        viewClass,
        "setOnClickListener",
        "(Landroid/view/View$OnClickListener;)V"
    );

    // Для простоты: при клике удаляем view через handler
    // В реальном проекте лучше создать отдельный класс слушателя
}

} // extern "C"
