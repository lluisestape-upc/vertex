# 🎛️ BasicCompressor

Un plugin de compresor de audio básico desarrollado en C++ utilizando el framework JUCE. Este procesador reduce el rango dinámico de la señal de audio entrante basándose en los parámetros estándar de compresión dinámica.



## 🎚️ Características y Controles

El plugin expone los siguientes parámetros de control:

* **Gain:** Ajusta la ganancia general de la señal de audio (rango de -50 dB a +10 dB).
* **Threshold:** Establece el umbral en decibelios a partir del cual el compresor empieza a atenuar la señal (rango de -60 dB a 0 dB).
* **Ratio:** Determina la proporción de reducción de ganancia que se aplica una vez que la señal supera el umbral (rango de 1.0 a 10.0).
* **Attack:** El tiempo en milisegundos que tarda el compresor en aplicar la reducción de ganancia (rango de 0.1 ms a 100 ms).
* **Release:** El tiempo en milisegundos que tarda el compresor en dejar de actuar cuando la señal cae por debajo del umbral (rango de 10 ms a 500 ms).

## 🖥️ Interfaz Gráfica (GUI)

La interfaz incluye elementos de monitorización visual:

* **Curva de Transferencia:** Una gráfica estática que se actualiza para reflejar la curva de compresión resultante de los parámetros *Threshold* y *Ratio*.
* **Vúmetros de Nivel:** Dos medidores (Input y Output) que muestran los niveles de la señal en tiempo real, calibrados de -60 dB a 0 dB.

## 🛠️ Tecnologías

* C++
* [JUCE Framework](https://juce.com/)

## 🚀 Cómo compilar

Para compilar este proyecto en Windows:

1. Asegúrate de tener instalado **JUCE** y **Visual Studio** (2019 o 2022) con las herramientas de desarrollo de escritorio en C++.
2. Abre el archivo `.jucer` del proyecto utilizando la aplicación **Projucer**.
3. Haz clic en el icono de Visual Studio en Projucer ("Save and Open in IDE") para generar los archivos de la solución.
4. En Visual Studio, selecciona la configuración que necesites (`Debug` o `Release`) y compila el proyecto (`Build > Build Solution`).
5. Encontrarás el plugin compilado (típicamente en formato VST3 o Standalone) dentro de la carpeta `Builds/VisualStudio...` en el directorio de tu proyecto.
