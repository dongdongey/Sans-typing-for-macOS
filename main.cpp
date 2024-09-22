#include <ApplicationServices/ApplicationServices.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <iostream>

// 오디오 파일 재생 함수
void PlaySound(const char* soundFilePath) {
    CFURLRef soundFileURL = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault,
        (const UInt8*)soundFilePath,
        strlen(soundFilePath),
        false
    );
    
    SystemSoundID soundID;
    AudioServicesCreateSystemSoundID(soundFileURL, &soundID);
    AudioServicesPlaySystemSound(soundID);
    
    CFRelease(soundFileURL);
}

// 애플리케이션 번들에서 리소스 파일 경로를 얻는 함수
std::string GetResourcePath(const char* fileName) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();  // 애플리케이션의 메인 번들 가져오기
    if (!mainBundle) {
        std::cerr << "메인 번들을 찾을 수 없습니다!" << std::endl;
        return "";
    }

    CFStringRef cfFileName = CFStringCreateWithCString(kCFAllocatorDefault, fileName, kCFStringEncodingUTF8);
    CFURLRef resourceURL = CFBundleCopyResourceURL(mainBundle, cfFileName, nullptr, nullptr);
    CFRelease(cfFileName);

    if (!resourceURL) {
        std::cerr << "리소스 파일을 찾을 수 없습니다: " << fileName << std::endl;
        return "";
    }

    // URL을 파일 경로 문자열로 변환
    char filePath[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourceURL, true, (UInt8*)filePath, sizeof(filePath))) {
        std::cerr << "리소스 파일 경로를 변환할 수 없습니다!" << std::endl;
        CFRelease(resourceURL);
        return "";
    }
    
    CFRelease(resourceURL);
    return std::string(filePath);
}

// 키보드 이벤트 콜백 함수
CGEventRef MyCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* userInfo) {
    if (type == kCGEventKeyDown) {
        CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        if (keyCode == 53) {
            std::cout << "ESC 키가 눌렸습니다. 앱을 종료합니다." << std::endl;
            // 앱 종료
            exit(0);
        }
        // 리소스 경로에서 음원 파일 가져오기
        std::string soundFilePath = GetResourcePath("SansTyping_SansSpeak.wav");
        if (!soundFilePath.empty()) {
            PlaySound(soundFilePath.c_str());
        }
    }
    
    return event;
}

int main() {
    // Quartz Event Tap을 사용하여 키보드 이벤트 캡처
    CGEventMask eventMask = (1 << kCGEventKeyDown); // 키다운 이벤트 감지
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionListenOnly,
        eventMask,
        MyCGEventCallback,
        nullptr
    );

    if (!eventTap) {
        std::cerr << "Event Tap을 생성할 수 없습니다!" << std::endl;
        return -1;
    }
    
    // Event Tap을 런루프에 추가
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    // 런루프 실행
    CFRunLoopRun();
    
    // 종료시 리소스 해제
    CFRelease(runLoopSource);
    CFRelease(eventTap);
    
    return 0;
}
