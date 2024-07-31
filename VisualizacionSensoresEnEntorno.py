import numpy as np
from stl import mesh
from vispy import app, scene
from vispy.scene import visuals

NumeroSensores=str(input("Ingresar N° Sensores en múltiplos de 5 [Límite 30]. : "))

# Función para leer las ubicaciones de los Sensores
def read_highlight_points(file_path):
    points = []
    with open(file_path, 'r') as file:
        for line in file:
            point = list(map(float, line.strip().split()))
            points.append(point)
    return np.array(points)

# Ruta al archivo de puntos
file_path = 'PuntosSensores'+NumeroSensores+'.txt'

# Leer los Sensores
highlight_points = read_highlight_points(file_path)

# Cargar el archivo STL
stl_mesh = mesh.Mesh.from_file('Entorno de Experimentos.stl')

# Extraer todos los vértices del STL
all_points = stl_mesh.vectors.reshape(-1, 3)

# Crear una lista para almacenar los colores de los puntos
normal_points = np.array([point for point in all_points if point.tolist() not in highlight_points.tolist()])
highlight_colors = np.array([[1, 0, 0, 1]] * highlight_points.shape[0])  # Rojo para puntos destacados
normal_colors = np.array([[1, 1, 1, 1]] * normal_points.shape[0])  # Blanco para puntos normales

# Crear una canvas de VisPy
canvas = scene.SceneCanvas(keys='interactive', show=True)
view = canvas.central_widget.add_view()

# Añadir los puntos normales a la visualización
scatter_normal = visuals.Markers()
scatter_normal.set_data(normal_points, face_color=normal_colors, size=3)
view.add(scatter_normal)

# Añadir los Sensores a la visualización
scatter_highlight = visuals.Markers()
scatter_highlight.set_data(highlight_points, face_color=highlight_colors, size=15)
view.add(scatter_highlight)

# Configurar la cámara
view.camera = 'turntable'  # 'turntable' o 'arcball'
view.camera.fov = 50
view.camera.distance = 500

# Ejecutar la aplicación
if __name__ == '__main__':
    app.run()