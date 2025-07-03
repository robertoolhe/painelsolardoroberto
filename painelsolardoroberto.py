# painelsolar.py® Criado por Roberto Olhê github.com/robertoolhe (Roberto La Bella) - Dashbord Painel Solar - Todos Direitos Reservados 2025®    
# painelsolar.py® Desenvolvido em jun/25 com Python+Pandas+StreamLit+Plotly
# substitui SQLalchemy por requests
# painelsolar.py® Utilizando VS Code com Sweet Dracula
# painelsolar.py® Utilizando Banco de Dados em Nuvem: PostgreSQL no Neon.tech
# substituí PostgreSQL Neon Tech pago por Banco de Dados SQLite no Google Drive

# painelsolar.py® Compartilhado no GitHub/robertoolhe (Roberto La Bella) e Streamlit Community Cloud
# painelsolar.py® Controle de Versões com GIT e testes com Git Bash
# painelsolar.py® Leituras do Sensor de Voltagem Desenvolvida no Esp32 TT-GO T-Display em MicroPython + NeonPostgresOverHTTPProxyClient
# docker instalado com wsl --install -d Ubuntu
# export HOME="/c/Users/tuneg" se o GIT não encontrar HOME
# executar com streamlit run painelsolar.py --server.port 8501 --server.address


import streamlit as st
import pandas as pd
import gspread
from oauth2client.service_account import ServiceAccountCredentials
import plotly.express as px
import plotly.graph_objects as go
import json

service_account_info = st.secrets["gcp_service_account"]
with open("service_account.json", "w") as f:
    json.dump(dict(service_account_info), f)

# Configuração do acesso ao Google Sheets
scope = ["https://spreadsheets.google.com/feeds", "https://www.googleapis.com/auth/drive"]
creds = ServiceAccountCredentials.from_json_keyfile_name("service_account.json", scope)
client = gspread.authorize(creds)

# Nome da planilha e da aba
SHEET_NAME = "painelsolardoroberto"  # Nome da sua planilha
WORKSHEET_NAME = "dados"             # Nome da aba (worksheet), ajuste conforme necessário

@st.cache_data(ttl=60)
def carregar_dados():
    sheet = client.open(SHEET_NAME).worksheet(WORKSHEET_NAME)
    data = sheet.get_all_records()
    df = pd.DataFrame(data)
    return df

df = carregar_dados()

# Converte 'data' para datetime
df['data'] = pd.to_datetime(df['data'], errors='coerce')
df['data'] = df['data'].dt.tz_localize(None)  # <-- Remove o timezone
df['voltagem'] = pd.to_numeric(df['voltagem'], errors='coerce')
df = df.dropna(subset=['data', 'voltagem'])
df['voltagem'] = df['voltagem'].astype(float)

# Filtra últimas 4 horas (LÊ ULTIMOS 240 minutos)
limite_tempo = pd.Timestamp.now() - pd.Timedelta(minutes=240)
# Filtra últimas horas e apenas 'nome' == 'voltagem'
df = df[(df['data'] >= limite_tempo) & (df['nome'] == 'voltagem')]

# Ordena por item ASC e data DESC
df = df.sort_values(by='data', ascending=False)

# Limita a 1000 linhas
df = df.head(1200)

# Cria coluna 'hora' igual ao SQL
df['hora'] = df['data'].dt.strftime('%H:%M')

# Exibe o DataFrame no console para depuração
print("Dados carregados do Google Sheets:") 
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
        <span class="by-dash">DashBoard</span> Painel Solar <span class="by-autor">by Roberto La Bella</span>
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


#st.line_chart(df.set_index('to_char').voltagem)
# Exibe o DataFrame no Streamlit
#st.write("Dados do Sensor de Voltagem")

# Crie uma coluna formatada só para exibição
df['voltagem_fmt'] = df['voltagem'].map(lambda x: f"{x:.2f}")

# Ordene o df para o gráfico (mais antigo para mais recente)
df_chart = df.sort_values(by='hora', ascending=True)

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
        df_chart, 
        x='hora', 
        y='voltagem',
        labels={'hora': 'Últimas 4 Horas', 'voltagem': 'Tensão (V)'},
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
    # Ordena por data/hora decrescente para pegar o mais recente
    df_gauge = df.sort_values(by='data', ascending=False)
    ultimo_valor = df_gauge['voltagem'].iloc[0]
    ultima_hora = df_gauge['hora'].iloc[0]
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
            'axis': {'range': [12.0, 14.4]},
            'bar': {'color': "lightgray"},
            'steps': [
                {'range': [12.0, 12.8], 'color': "indigo"}, 
                {'range': [12.8, 13.8], 'color': "purple"},
                {'range': [13.8, 14.4], 'color': "violet"}
            ],
        }
    ))
    fig_gauge.update_layout(margin=dict(t=4, b=0, l=0, r=0), height=380)
    st.plotly_chart(fig_gauge, use_container_width=True)

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
    df_donut = contagem.reset_index()
    df_donut.columns = ['Voltagem', 'Frequência']
    df_donut['Voltagem'] = df_donut['Voltagem'].astype(float).map(lambda x: f"{x:.2f}")

    # Mostra só as 5 voltagens mais frequentes
    df_donut = df_donut.head(5)

    if not df_donut.empty:
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
    else:
        st.info("Não há dados suficientes para exibir o gráfico de rosca.")

# Para exibição invertida na tabela:
with col_df:
    df_exibe = df.sort_values(by='data', ascending=False)[['hora', 'voltagem_fmt', 'data']].copy()
    df_exibe = df_exibe.rename(columns={'hora': 'Hora', 'voltagem_fmt': 'Tensão (V)'})
    df_exibe = df_exibe.head(5)
    st.dataframe(
        df_exibe[['Hora', 'Tensão (V)']],
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
    range=[12.8, 14.0],   # Defina o mínimo e máximo desejado
    dtick=0.1,
    tickformat=".2f"
    )
    fig_area.update_layout(margin=dict(t=20, b=20, l=0, r=0))
    st.plotly_chart(fig_area, use_container_width=True)

# Atualiza a cada 30 segundos
#st.write("A página será atualizada automaticamente a cada 30 segundos.")
#st.rerun()

st.markdown(
    """
    <meta http-equiv="refresh" content="60">
    """,
    unsafe_allow_html=True
)


