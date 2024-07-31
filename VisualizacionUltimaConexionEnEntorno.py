import numpy as np
from stl import mesh
from vispy import app, scene
from vispy.scene import visuals

# Cargar el archivo STL
stl_mesh = mesh.Mesh.from_file('Entorno de Experimentos.stl')

# Extraer todos los vértices del STL
all_points = stl_mesh.vectors.reshape(-1, 3)

# Cargar el archivo de texto
file_path = 'ResultadoUltimaEjecucion.txt'

transmisores = []
conexiones = []

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

# Crear una canvas de VisPy
canvas = scene.SceneCanvas(keys='interactive', show=True)
view = canvas.central_widget.add_view()

# Añadir los puntos STL a la visualización
stl_colors = np.array([[1, 1, 1, 0.5]] * all_points.shape[0])  # Blanco para los puntos STL
scatter_stl = visuals.Markers()
scatter_stl.set_data(all_points, face_color=stl_colors, size=2)
view.add(scatter_stl)

# Añadir los transmisores a la visualización
scatter_transmisores = visuals.Markers()
scatter_transmisores.set_data(transmisores, face_color='red', size=15)
view.add(scatter_transmisores)

# Añadir las conexiones a la visualización
for (start, end) in conexiones:
    x = [transmisores[start, 0], transmisores[end, 0]]
    y = [transmisores[start, 1], transmisores[end, 1]]
    z = [transmisores[start, 2], transmisores[end, 2]]
    line = visuals.Line(pos=np.array([[x[0], y[0], z[0]], [x[1], y[1], z[1]]]), color=[0, 0, 1, 0.8], width=3)
    view.add(line)

# Configurar la cámara
view.camera = 'turntable'  # 'turntable' o 'arcball'
view.camera.fov = 50
view.camera.distance = 500

# Ejecutar la aplicación
if __name__ == '__main__':
    app.run()