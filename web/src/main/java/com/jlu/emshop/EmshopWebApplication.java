package com.jlu.emshop;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * JLU Emshop Web应用程序启动类
 */
@SpringBootApplication
public class EmshopWebApplication {

    static {
        // 加载本地JNI库
        try {
            System.loadLibrary("emshop_native_oop");
            System.out.println("JNI库加载成功");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("JNI库加载失败: " + e.getMessage());
            // 在开发环境中，可以继续运行，使用模拟数据
        }
    }

    public static void main(String[] args) {
        SpringApplication.run(EmshopWebApplication.class, args);
    }
}
