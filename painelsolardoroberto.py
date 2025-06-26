# painelsolar.py® Criado por Roberto Olhê github.com/robertoolhe - Dashbord Painel Solar - Todos Direitos Reservados 2025®    
# painelsolar.py® Desenvolvido em jun/25 com Python+Pandas+StreamLit+Plotly
# substitui SQLalchemy por requests
# painelsolar.py® Utilizando VS Code com Sweet Dracula
# painelsolar.py® Utilizando Banco de Dados em Nuvem: PostgreSQL no Neon.tech
# substituí PostgreSQL Neon Tech pago por Banco de Dados SQLite no Google Drive
# link share: https://drive.google.com/file/d/1QnRvLKTc1sZyqxVODPehOF55sYPyhZHz/view?usp=drive_link
# ID: 1QnRvLKTc1sZyqxVODPehOF55sYPyhZHz


# painelsolar.py® Compartilhado no GitHub/robertoolhe e Streamlit Community Cloud
# painelsolar.py® Controle de Versões com GIT e testes com Git Bash
# painelsolar.py® Leituras do Sensor de Voltagem Desenvolvida no Esp32 TT-GO T-Display em MicroPython + NeonPostgresOverHTTPProxyClient
# docker instalado com wsl --install -d Ubuntu
# export HOME="/c/Users/tuneg" se o GIT não encontrar HOME
# executar com streamlit run painelsolar.py --server.port 8501 --server.address


#import psycopg2 as pg
import streamlit as st
import pandas as pd
import sqlite3
import requests
import time
#from sqlalchemy import create_engine
#from sqlalchemy import text
import plotly.express as px
import plotly.graph_objects as go #plotly.graph_objects é usado para criar gráficos mais complexos, como 2 colunas

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

# consulta SQL 
# consulta SQL adaptada para SQLite
sql = (
    "SELECT substr(measure_time, 12, 5) as hora, voltagem, item "
    "FROM dados "
    "WHERE sensor_name = 'voltagem' "
    "AND measure_time >= datetime('now', '-480 minutes') "
    "ORDER BY item ASC, measure_time DESC "
    "LIMIT 1000"
)

df = pd.read_sql_query(sql, conn)
conn.close()

print(df)

# Configurações do Streamlit
st.set_page_config(page_title="Painel de Monitoramento de Sistema Solar", layout="wide")
# Título da página
st.markdown(
    """
    <link href="https://fonts.cdnfonts.com/css/nasa" rel="stylesheet">
    <style>
    h1 {
        font-family: 'NASA', Arial, sans-serif !important;
        color: lightgray !important;
        text-align: center !important;
        width: 100%;
        display: block;
        margin-top: -60px !important;
        padding-top: 0 !important;
        margin-bottom: 0 !important;    /* adicione esta linha */
        padding-bottom: 0 !important;   /* adicione esta linha */
    }
    .by-dash {
        color: gray !important;
        font-size: 0.8em;
        font-weight: normal;
    }
    .by-autor {
        color: gray !important;
        font-size: 0.8em;
        font-weight: normal;
    }
    </style>
    <h1>
        <span class="by-dash">DashBoard</span> Painel Solar <span class="by-autor">by Roberto Olhê</span>
    </h1>
    """,
    unsafe_allow_html=True
)
#

# Configura o estilo do Streamlit
st.markdown(
    """
    <style>    .stApp {
        background-color: #000000;
    }
    </style>
    """,
    unsafe_allow_html=True
)
# se quiser cinza use a color #f0f2f5;

# Plota o gráfico de linha
# Converta a coluna 'hora' para datetime apenas com hora e minuto
df['hora'] = pd.to_datetime(df['hora'], format='%H:%M', errors='coerce')
df['voltagem'] = pd.to_numeric(df['voltagem'], errors='coerce')
df = df.dropna(subset=['voltagem'])
df['voltagem'] = df['voltagem'].astype(float)
df['hora'] = df['hora'].dt.strftime('%H:%M')
# Plota o gráfico de linha com o eixo x formatado
#st.subheader("Últimas 4 horas - Dados coletados a cada 30 segundos")
#st.write("Últimas 4 horas - Dados coletados a cada 30 segundos")
#st.text("| Últimas horas | Dados coletados a cada 30 segundos |")
# Cria o layout com duas colunas
# Crie duas colunas
col_chart, col_gauge = st.columns([2, 1])  # Ajuste a proporção conforme desejar

with col_chart:
    # Gráfico de linhas
    fig = px.line(
        df, 
        x='hora', 
        y='voltagem',
        labels={'hora': 'Últimas 8 Horas', 'voltagem': 'Tensão (V)'},
        height=380,  # Defina a altura desejada em pixels
        color_discrete_sequence=['violet']
    )
    fig.update_yaxes(
        #range=[12.30, 14.40],
        dtick=0.1,
        tickformat=".2f"
    )
    st.plotly_chart(fig, use_container_width=True)
    # Adiciona uma linha horizontal em y=2.80 com cor personalizada
    fig.add_hline(
        y=2.80,
        line_dash="dash",
        line_color="violet",  # Escolha a cor desejada
        line_width=2,
        annotation_text="2.80V",
        annotation_position="top left"
    )

with col_gauge:
    # Gráfico gauge com o último valor de voltagem
    df = df.rename(columns={'hora': 'hora'})
    ultimo_valor = df['voltagem'].iloc[-1]
    ultima_hora = df['hora'].iloc[-1]  # após o rename, a coluna é 'hora'
    fig_gauge = go.Figure(go.Indicator(
        mode="gauge+number",
        value=ultimo_valor,
        number={
            'valueformat': '.2f',
            'font': {'size': 76},
            'suffix': " V<br>{}".format(ultima_hora) 
        },
        #title={'text': "Voltagem Atual (V)"},
        gauge={
            'axis': {'range': [12.3, 14.5]},
            'bar': {'color': "lightgray"},
            'steps': [
                {'range': [12.6, 13.0], 'color': "indigo"}, 
                {'range': [13.0, 14.0], 'color': "purple"},
                {'range': [14.0, 14.5], 'color': "violet"}
            ],
        }
    ))
    fig_gauge.update_layout(margin=dict(t=4, b=0, l=0, r=0), height=380)
    st.plotly_chart(fig_gauge, use_container_width=True)


#st.line_chart(df.set_index('to_char').voltagem)
# Exibe o DataFrame no Streamlit
#st.write("Dados do Sensor de Voltagem")

df['voltagem'] = df['voltagem'].map(lambda x: f"{x:.2f}")
df = df.sort_values(by='hora', ascending=False)  # 'hora' é o nome da coluna após o rename
#st.dataframe(df)

# Cria uma coluna arredondada para a voltagem

df['voltagem_arred'] = df['voltagem'].astype(float).round(2)
contagem = df['voltagem_arred'].value_counts().sort_values(ascending=False)
percentual = contagem / contagem.sum()
percentual_acumulado = percentual.cumsum()
volts_40 = percentual_acumulado[percentual_acumulado <= 0.5].index
contagem_40 = contagem.loc[volts_40]

# Cria duas colunas: gráfico bolhas (1x) e DataFrame (2x) e Donut (2x)
col_donut , col_df, col_chart_min = st.columns([1, 0.7, 2]) 

with col_donut:
    # Gráfico de rosca das voltagens mais frequentes (top 40%)
    df_donut = contagem_40.reset_index()
    df_donut.columns = ['Voltagem', 'Frequência']
    df_donut['Voltagem'] = df_donut['Voltagem'].astype(float).map(lambda x: f"{x:.2f}")

    # Mostra só as 5 voltagens mais frequentes
    df_donut = df_donut.head(5)

    fig_donut = px.pie(
        df_donut,
        names='Voltagem',
        values='Frequência',
        hole=0.5,
        color_discrete_sequence=['purple', 'violet', 'indigo', 'plum', 'orchid']
    )
    fig_donut.update_traces(textinfo='percent+label')
    fig_donut.update_layout(height=220, margin=dict(t=20, b=20, l=0, r=0))
    st.plotly_chart(fig_donut, use_container_width=True)

with col_df:
    df.drop(columns=['voltagem_arred'], inplace=True)
    df = df.replace("None", "")
    df_exibe = df.head(5) # Limita o DataFrame a 5 linhas para exibição
    df_exibe = df_exibe.rename(columns={'hora': 'Hora', 'voltagem': 'Tensão (V)'})
    df_exibe = df_exibe.iloc[:, :2] #df_exibe = df_exibe[['Hora', 'Tensão (V)']]
    #df = df.iloc[::-1] # Exemplo de: Inverte para ordem cronológica (mais antigo primeiro)
    st.dataframe(
        df_exibe,
        height=220,
        column_config={
            "Hora": st.column_config.Column(width="small"),
            "Tensão (V)": st.column_config.Column(width="small")
        }
    )


with col_chart_min:
    df['hora_dt'] = pd.to_datetime(df['hora'], format='%H:%M')
    ultimo_horario = df['hora_dt'].max()
    minutos_15 = ultimo_horario - pd.Timedelta(minutes=15)
    df_area = df[df['hora_dt'] >= minutos_15]
    df_area = df_area.sort_values(by='hora_dt')

    fig_area = px.area(
        df_area,
        x='hora',
        y='voltagem',
        labels={'hora': 'Últimos 15 minutos', 'voltagem': 'Tensão (V)'},
        color_discrete_sequence=['violet'],
        height=220
    )
    fig_area.update_yaxes(
    range=[12.3, 14.2],   # Defina o mínimo e máximo desejado
    dtick=0.1,
    tickformat=".2f"
    )
    fig_area.update_layout(margin=dict(t=20, b=20, l=0, r=0))
    st.plotly_chart(fig_area, use_container_width=True)

# Atualiza a cada 30 segundos
#st.write("A página será atualizada automaticamente a cada 30 segundos.")
time.sleep(30)
st.rerun()
