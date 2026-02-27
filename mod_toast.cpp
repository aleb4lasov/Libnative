#include <jni.h>
#include <android/log.h>
#include <string>

// Тэг для логов (будет видно в LogCat)
#define TAG "ModdedByAlebalasov"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

extern "C" {

/**
 * JNI-функция, которая будет вызвана из Java-кода мода.
 * Обратите внимание на имя: Java_имя_пакета_класса_метод
 * Мы назовем класс NativeMod, а пакет com.mod
 */
JNIEXPORT void JNICALL
Java_com_mod_NativeMod_showCustomToast(JNIEnv *env, jobject thiz, jobject context) {
    
    // Логируем вызов для отладки
    LOGD("showCustomToast called from JNI!");
    
    // 1. Находим класс Toast
    jclass toastClass = env->FindClass("android/widget/Toast");
    if (toastClass == nullptr) {
        LOGD("Failed to find Toast class");
        return;
    }
    
    // 2. Получаем метод makeText(context, charSequence, duration)
    jmethodID makeTextMethod = env->GetStaticMethodID(
        toastClass,
        "makeText",
        "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"
    );
    if (makeTextMethod == nullptr) {
        LOGD("Failed to find makeText method");
        return;
    }
    
    // 3. Получаем метод show()
    jmethodID showMethod = env->GetMethodID(toastClass, "show", "()V");
    if (showMethod == nullptr) {
        LOGD("Failed to find show method");
        return;
    }
    
    // 4. Создаем Java-строку с нашим сообщением
    jstring message = env->NewStringUTF("Modded by alebalasov");
    
    // 5. Вызываем Toast.makeText(context, message, Toast.LENGTH_SHORT).show()
    jobject toast = env->CallStaticObjectMethod(
        toastClass,
        makeTextMethod,
        context,      // Используем контекст, переданный из Java
        message,
        0             // 0 = LENGTH_SHORT
    );
    
    if (toast != nullptr) {
        env->CallVoidMethod(toast, showMethod);
        LOGD("Toast shown successfully");
    } else {
        LOGD("Failed to create Toast object");
    }
    
    // 6. Очищаем локальные ссылки (хороший тон)
    env->DeleteLocalRef(message);
    env->DeleteLocalRef(toast);
    // Не удаляем toastClass, т.к. это глобальный класс, но для простоты оставим
}

} // extern "C"