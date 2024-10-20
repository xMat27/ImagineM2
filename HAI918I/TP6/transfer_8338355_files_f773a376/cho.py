import glfw
from OpenGL.GL import *
from OpenGL.GLU import *
import numpy as np
import math
import struct


# Structure pour un sommet (3D)
class Vertex:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x = x
        self.y = y
        self.z = z


# Structure pour le maillage
class Mesh:
    def __init__(self):
        self.vertices = []


class VertexWithRadius:
    def __init__(self, v, radius):
        self.v = v
        self.radius = radius
        self.azimuth = 0.0
        self.elevation = 0.0


# Fonction pour calculer le barycentre du maillage
def compute_centroid(mesh):
    centroid = Vertex()
    num_vertices = len(mesh.vertices)
    
    for v in mesh.vertices:
        centroid.x += v.x
        centroid.y += v.y
        centroid.z += v.z
    
    centroid.x /= num_vertices
    centroid.y /= num_vertices
    centroid.z /= num_vertices
    return centroid


# Convertir un sommet des coordonnées cartésiennes aux coordonnées polaires
def cartesian_to_polar(v, centroid):
    dx = v.x - centroid.x
    dy = v.y - centroid.y
    dz = v.z - centroid.z

    # Calcul du rayon (distance entre le sommet et le barycentre)
    radius = math.sqrt(dx ** 2 + dy ** 2 + dz ** 2)

    # Calcul de l'azimut (angle autour de l'axe z)
    azimuth = math.acos(dz / radius) if radius != 0 else 0.0

    # Calcul de l'élévation (angle vertical par rapport au plan xy)
    elevation = math.atan2(dy, dx)  # atan2(y, x)

    return radius, azimuth, elevation


# Convertir des coordonnées polaires en coordonnées cartésiennes
def polar_to_cartesian(radius, azimuth, elevation, centroid):
    v = Vertex()
    v.x = centroid.x + radius * math.sin(elevation) * math.cos(azimuth)
    v.y = centroid.y + radius * math.sin(elevation) * math.sin(azimuth)
    v.z = centroid.z + radius * math.cos(elevation)  # Note: using cos for z coordinate
    return v


# Fonction d'initialisation de GLFW et création de fenêtre
def init_opengl_window(width, height, title):
    if not glfw.init():
        print("Erreur : Impossible d'initialiser GLFW")
        return None

    # Créer la fenêtre
    window = glfw.create_window(width, height, title, None, None)
    if not window:
        print("Erreur : Impossible de créer la fenêtre GLFW")
        glfw.terminate()
        return None

    glfw.make_context_current(window)
    glViewport(0, 0, width, height)
    return window


def draw_mesh(mesh):
    glBegin(GL_POINTS)  # Commencer à dessiner des points
    for v in mesh.vertices:
        glVertex3f(v.x, v.y, v.z)  # Placer chaque sommet
    glEnd()  # Terminer de dessiner


def text_to_binary(text):
    return ''.join(format(ord(c), '08b') for c in text)


def binary_to_text(binary_message):
    text = ''
    for i in range(0, len(binary_message), 8):
        byte = binary_message[i:i + 8]
        text += chr(int(byte, 2))
    return text


# Fonction pour charger un fichier .obj
def load_obj_mesh(filename):
    mesh = Mesh()
    
    try:
        with open(filename, 'r') as file:
            for line in file:
                if line.startswith('v '):
                    parts = line.split()
                    v = Vertex(float(parts[1]), float(parts[2]), float(parts[3]))
                    mesh.vertices.append(v)
    except IOError:
        print(f"Erreur : Impossible d'ouvrir le fichier {filename}")
        return None
    
    return mesh


# Fonction pour charger un fichier .off
def load_off_mesh(filename):
    mesh = Mesh()
    
    try:
        with open(filename, 'r') as file:
            line = file.readline()
            if line.strip() != "OFF":
                print("Erreur : Le fichier n'est pas un fichier OFF valide.")
                return None
            
            counts = file.readline().strip().split()
            num_vertices = int(counts[0])
            num_faces = int(counts[1])
            num_edges = int(counts[2])

            # Lire les sommets
            for _ in range(num_vertices):
                parts = file.readline().strip().split()
                v = Vertex(float(parts[0]), float(parts[1]), float(parts[2]))
                mesh.vertices.append(v)
    except IOError:
        print(f"Erreur : Impossible d'ouvrir le fichier {filename}")
        return None

    return mesh


def embed_message_in_mesh(mesh, message):
    centroid = compute_centroid(mesh)
    binary_message = text_to_binary(message)
    alpha = 0.0001
    num_classes = len(binary_message)

    vertices_with_radius = []
    
    # Transformation des sommets en coordonnées polaires
    for v in mesh.vertices:
        radius, azimuth, elevation = cartesian_to_polar(v, centroid)
        vertices_with_radius.append(VertexWithRadius(v, radius))

    # Trouver le rayon minimum et maximum
    min_radius = min(vertex.radius for vertex in vertices_with_radius)
    max_radius = max(vertex.radius for vertex in vertices_with_radius)

    # Si le min_radius et max_radius sont égaux ou trop proches, éviter la normalisation
    if abs(max_radius - min_radius) < 1e-6:
        print("Rayons trop proches, éviter la normalisation.")
        return

    # Normaliser les valeurs de radius
    for vertex in vertices_with_radius:
        vertex.radius = (vertex.radius - min_radius) / (max_radius - min_radius)

    vertices_with_radius.sort(key=lambda vertex: vertex.radius)

    vertices_per_class = len(vertices_with_radius) // num_classes

    # Encoder le message binaire dans les sommets
    for i in range(num_classes):
        start_idx = i * vertices_per_class
        end_idx = (i + 1) * vertices_per_class

        # Calculer la moyenne du radius pour cette classe
        average_radius = sum(vertex.radius for vertex in vertices_with_radius[start_idx:end_idx]) / vertices_per_class

        # Pour éviter une boucle infinie, ajout d'une limite au nombre d'itérations
        max_iterations = 10
        iteration_count = 0

        # Encoder le bit
        if binary_message[i] == '1':
            while average_radius <= 0.5 + alpha and iteration_count < max_iterations:
                iteration_count += 1
                for j in range(start_idx, end_idx):
                    vertices_with_radius[j].radius *= (1 + alpha)
                average_radius = sum(vertex.radius for vertex in vertices_with_radius[start_idx:end_idx]) / vertices_per_class

        else:
            while average_radius >= 0.5 - alpha and iteration_count < max_iterations:
                iteration_count += 1
                for j in range(start_idx, end_idx):
                    vertices_with_radius[j].radius *= (1 - alpha)
                average_radius = sum(vertex.radius for vertex in vertices_with_radius[start_idx:end_idx]) / vertices_per_class

        # Si on atteint max_iterations, on sort pour éviter une boucle infinie
        if iteration_count == max_iterations:
            print(f"Attention : Limite d'itérations atteinte pour le bit {i}.")

    # Dénormaliser et reconvertir les coordonnées polaires en coordonnées cartésiennes
    for vertex in vertices_with_radius:
        vertex.radius = vertex.radius * (max_radius - min_radius) + min_radius
        vertex.v = polar_to_cartesian(vertex.radius, vertex.azimuth, vertex.elevation, centroid)

    # Mise à jour du maillage avec les nouveaux sommets
    for i in range(len(mesh.vertices)):
        mesh.vertices[i] = vertices_with_radius[i].v



def extract_message_from_mesh(mesh, original_mesh, message_length):
    binary_message = ''
    epsilon_threshold = 0.00001  # Plus petit que epsilon utilisé dans l'insertion

    for i in range(message_length * 8):
        v = mesh.vertices[i]
        v_original = original_mesh.vertices[i]

        displacement = v.x - v_original.x
        if displacement > epsilon_threshold:
            binary_message += '1'
        elif displacement < -epsilon_threshold:
            binary_message += '0'
        else:
            print(f"Erreur : Aucun déplacement détecté à l'indice {i}.")
            return ""

    return binary_to_text(binary_message)


def main():
    mesh_original = load_off_mesh("monkey.off")
    if mesh_original is None:
        return

    mesh_with_message = mesh_original

    message = "hola"

    embed_message_in_mesh(mesh_with_message, message)

    extracted_message = extract_message_from_mesh(mesh_with_message, mesh_original, len(message))

    window = init_opengl_window(800, 600, "Visualisation du maillage")
    if not window:
        return

    # Boucle principale d'affichage
    while not glfw.window_should_close(window):
        # Effacer l'écran
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()

        # Dessiner le maillage
        draw_mesh(mesh_with_message)

        # Échanger les buffers
        glfw.swap_buffers(window)

        # Vérifier les événements
        glfw.poll_events()

    glfw.terminate()
    print(f"Message extrait : {extracted_message}")


if __name__ == "__main__":
    main()
