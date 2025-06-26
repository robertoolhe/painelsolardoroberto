import sqlite3
import requests
import streamlit as st
from pydrive.auth import GoogleAuth
from pydrive.drive import GoogleDrive


# Caminho do banco de dados (ajuste se necessário)
#db_path = "painelsolar.db"

# Baixar o arquivo .db do Google Drive (substitua pelo seu ID)
FILE_ID = "1QnRvLKTc1sZyqxVODPehOF55sYPyhZHz"  # <-- coloque o ID do seu arquivo aqui
DOWNLOAD_URL = f"https://drive.google.com/uc?export=download&id={FILE_ID}"

# Baixar o arquivo do Google Drive
@st.cache_data(ttl=60)  # cache para evitar downloads repetidos em pouco tempo
def baixar_db():
    r = requests.get(DOWNLOAD_URL)
    with open("painelsolar.db", "wb") as f:
        f.write(r.content)
    return "painelsolar.db"

db_path = baixar_db()
# Conecte ao banco SQLite
conn = sqlite3.connect(db_path)

gauth = GoogleAuth()
gauth.LocalWebserverAuth()
drive = GoogleDrive(gauth)

# Registro de teste (ajuste os valores conforme sua tabela)
novo_registro = (
    "2025-06-25 20:47:00.000000-03",  # measure_time
    12.97,                            # voltagem
    30004,                              # item
    "voltagem"                        # sensor_name
)

conn = sqlite3.connect(db_path)
cursor = conn.cursor()
cursor.execute(
    "INSERT INTO dados (measure_time, voltagem, item, sensor_name) VALUES (?, ?, ?, ?)",
    novo_registro
)
conn.commit()
conn.close()
print("Registro inserido com sucesso!")

# Cria um novo arquivo no Drive (ou substitui se já existir)
# Verifica se o arquivo já existe no Google Drive
file_list = drive.ListFile({'q': "title='painelsolardoroberto.db' and trashed=false"}).GetList()
if file_list:
    # Se já existe, atualiza
    file1 = file_list[0]
    file1.SetContentFile(db_path)
    file1.Upload()
    print("Arquivo atualizado no Google Drive!")
else:
    # Se não existe, cria novo
    file1 = drive.CreateFile({'title': 'painelsolardoroberto.db'})  # , 'parents':[{'id': folder_id}]
    file1.SetContentFile(db_path)
    file1.Upload()
    print("Arquivo enviado para o Google Drive!")


