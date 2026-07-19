import sys
import face_recognition

reference_path = "reference.jpg"
new_frame_path = sys.argv[1]

reference_image = face_recognition.load_image_file(reference_path)
reference_encodings = face_recognition.face_encodings(reference_image)

if len(reference_encodings) == 0:
    print("no_face_in_reference")
    sys.exit(1)

reference_encoding = reference_encodings[0]

new_image = face_recognition.load_image_file(new_frame_path)
new_encodings = face_recognition.face_encodings(new_image)

if len(new_encodings) == 0:
    print("no_match")
    sys.exit(0)

results = face_recognition.compare_faces([reference_encoding], new_encodings[0], tolerance=0.6)

if results[0]:
    print("match")
else:
    print("no_match")