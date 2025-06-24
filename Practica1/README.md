Proyecto: Búsqueda eficiente de casos COVID-19 en Colombia
Materia: Sistemas Operativos
Integrantes: 

🧾 Descripción general

Este proyecto implementa un sistema de búsqueda eficiente sobre un dataset grande (6,3 millones de registros) de casos positivos de COVID-19 en Colombia. La búsqueda se realiza por nombre del departamento, edad y sexo, permitiendo uno o varios criterios a la vez. La solución se construyó en lenguaje C, cumpliendo los requisitos de uso limitado de memoria (<10 MB), tiempo de respuesta (<2 s) y procesos no emparentados comunicados mediante tuberías nombradas (FIFO).

📂 Archivos entregados

    interfaz.c → Proceso que gestiona el menú y la entrada del usuario

    buscador.c → Proceso que realiza la búsqueda eficiente y retorna resultados

    hash_indexador.c → Crea los archivos binarios indexados depto_index.bin y depto_lists.bin

    dataset.csv → Archivo de datos (no incluido por tamaño)

    depto_index.bin, depto_lists.bin → Archivos binarios generados para búsqueda eficiente

    Makefile → Automatiza la compilación

    README.md→ Este archivo

📊 Dataset utilizado

Nombre: Casos positivos de COVID-19 en Colombia
Filas: 6.390.971
Columnas: 23
Campos utilizados:

    Nombre del departamento (columna 5)

    Edad (columna 8)

    Sexo (columna 10)

🧠 Justificación de los criterios de búsqueda
    
    Nombre del departamento:
    El departamento representa la unidad territorial principal en Colombia y es fundamental para realizar análisis geográficos. Filtrar por este campo permite identificar patrones regionales, como brotes localizados o zonas con mayor número de casos activos, fallecimientos o recuperación. También es útil para que las autoridades locales puedan tomar decisiones con base en datos específicos de su región, como asignar recursos médicos, planificar campañas de vacunación o controlar la movilidad.

    Edad:
    La edad de las personas afectadas por COVID-19 es un indicador epidemiológico clave. Los efectos del virus varían significativamente entre grupos etarios: los adultos mayores, por ejemplo, presentan mayor riesgo de hospitalización o muerte, mientras que los jóvenes pueden ser transmisores asintomáticos. Al incluir la edad como criterio de búsqueda, el sistema permite generar reportes más específicos por grupo poblacional, lo que resulta fundamental para diseñar políticas de salud pública más enfocadas y equitativas. Además, permite detectar cambios en las tendencias a lo largo del tiempo.

    Sexo:
    El sexo también influye en cómo afecta el virus a las personas. Algunos estudios han encontrado diferencias en tasas de contagio, mortalidad o comorbilidades entre hombres y mujeres. Al permitir filtrar por sexo, este sistema ofrece la posibilidad de detectar esas diferencias en los datos, lo que puede ser útil tanto para investigaciones académicas como para mejorar la equidad en la atención médica. También permite combinar filtros para ver, por ejemplo, si ciertas edades o regiones presentan disparidades según el género.

🎯 Estructura del sistema

    hash_indexador: Lee dataset.csv y genera archivos binarios indexados por nombre de departamento:

        depto_index.bin: índice principal (hash, nombre, cantidad de offsets, posición)

        depto_lists.bin: listas de offsets por departamento

    buscador_ipc:

        Espera criterios de búsqueda desde entrada_fifo

        Si se da un departamento, usa búsqueda por índice; si no, recorre todo el archivo

        Filtra por edad y sexo si están presentes

        Devuelve resultados y estadísticas por salida_fifo

    interfaz:

        Presenta menú interactivo al usuario

        Permite modificar cada criterio individualmente

        Solo requiere 1 de los 3 criterios para buscar

        Envia búsqueda al buscador y muestra resultados

📌 Validaciones implementadas

    Edad: debe estar entre 0 y 110 años

    Sexo: solo se aceptan “M” o “F”

    Departamento: permite espacios, y se envía entre comillas para evitar errores de formato

    Si no se encuentran resultados, se imprime “NA”

⚙️ Instrucciones de uso

    Compilar todos los programas:
    make

    Generar archivos de índice:
    ./hash_indexador

    Ejecutar buscador en segundo plano (terminal A):
    ./buscador_ipc

    Ejecutar interfaz (terminal B):
    ./interfaz

    El menú permite ingresar criterios y ejecutar la búsqueda.

    Limpiar archivos generados:
    make clean

🧠 Requerimientos técnicos cumplidos

    ✔ Procesos no emparentados

    ✔ Comunicación por FIFO

    ✔ Búsqueda optimizada con tabla hash

    ✔ Estructuras dinámicas (malloc/free)

    ✔ Uso de memoria RAM < 10 MB (medido)

    ✔ Tiempo de respuesta < 2 segundos (medido)

    ✔ Formato de menú y validaciones según lo solicitado
