import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# Leer el archivo de texto
file_path = 'ResultadoUltimaEjecucion.txt'

transmisores = []
conexiones = []

try:
    with open(file_path, 'r') as file:
        lines = file.readlines()
        
        # Procesar ubicaciones de los transmisores
        line_index = 0
        while "Transmisores" not in lines[line_index]:
            line_index += 1
        line_index += 1
        while "Conexiones" not in lines[line_index]:
            line = lines[line_index].strip()
            if line.startswith("Transmisor"):
                try:
                    parts = line.split(':')[1].strip().split()
                    coord = list(map(float, parts))
                    transmisores.append(coord)
                except ValueError as e:
                    print(f"Error al convertir coordenadas '{line}': {e}")
            line_index += 1
        
        # Procesar conexiones
        line_index += 1
        while line_index < len(lines):
            line = lines[line_index].strip()
            if line and "Conectado a" in line:
                try:
                    parts = line.split()
                    start = int(parts[0])
                    end = int(parts[3])
                    conexiones.append((start, end))
                except (IndexError, ValueError) as e:
                    print(f"Error al procesar la línea '{line}': {e}")
            line_index += 1

    transmisores = np.array(transmisores)

    # Configuración de la visualización
    fig = plt.figure(dpi=75)
    ax = fig.add_subplot(111, projection='3d')

    # Plotear los transmisores
    ax.scatter(transmisores[:, 0], transmisores[:, 1], transmisores[:, 2], c='r', marker='o')

    # Añadir etiquetas a los transmisores
    for i, (x, y, z) in enumerate(transmisores):
        ax.text(x, y, z, f'T{i}', color='black')

    # Plotear las conexiones
    for (start, end) in conexiones:
        x = [transmisores[start, 0], transmisores[end, 0]]
        y = [transmisores[start, 1], transmisores[end, 1]]
        z = [transmisores[start, 2], transmisores[end, 2]]
        ax.plot(x, y, z, color='b')

    # Configuración de los ejes
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')

    # Ajustar los límites de los ejes
    x_min, x_max = transmisores[:, 0].min() - 10, transmisores[:, 0].max() + 10
    y_min, y_max = transmisores[:, 1].min() - 10, transmisores[:, 1].max() + 10
    z_min, z_max = transmisores[:, 2].min() - 10, transmisores[:, 2].max() + 10

    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    ax.set_zlim(z_min, z_max)

    # Desactivar la cuadrícula y los ejes
    ax.grid(False)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_zticks([])

    plt.title('Visualización 3D de las ubicaciones y conexiones de los Transmisores.')
    plt.show()

except Exception as e:
    print(f"Error al procesar el archivo: {e}")