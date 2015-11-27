package eu.slomkowski.szark.client;

import com.jdotsoft.jarloader.JarClassLoader;

public class Launcher {

    public static void main(String[] args) {
        JarClassLoader jcl = new JarClassLoader();

        try {
            jcl.invokeMain("eu.slomkowski.szark.client.SzarkClient", args);
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }
}
