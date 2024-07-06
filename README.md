# SymbolNodeViewer

## 概要

SymbolNodeViewerは、M5Stack Core2を使用して、Symbolブロックチェーンのノード状態をリアルタイムで監視するためのツールです。このプロジェクトは、ブロック高を定期的に取得し、M5Stack Core2の画面に表示します。
開発にはPlatformIoを使用しています。

## 特徴

- Symbolブロックチェーンのノード状態をリアルタイムで監視
- ブロック高の定期的な更新（40秒ごと）
- 更新状態に応じた画面色の変更（通常：黒、40秒以上更新なし：黄、5分以上更新なし：赤）
- 設定情報をSDカードから読み込み
- NTPサーバーを使用した時刻同期

## 必要なハードウェア

- M5Stack Core2
- SDカード

## 必要なライブラリ

- M5Unified
- WiFi
- HTTPClient
- ArduinoJson
- SD
- Time

## セットアップ

1. このリポジトリをクローンまたはダウンロードします。

2. 必要なライブラリをインストールします。

3. SDカードに `config.txt` ファイルを作成し、以下の形式で設定を記述します：

4. SDカードをM5Stack Core2に挿入します。

5. プロジェクトをコンパイルし、M5Stack Core2に書き込みます。

## SDカードの内容
ssid=YOUR_WIFI_SSID  
password=YOUR_WIFI_PASSWORD  
api_url=YOUR_API_URL + /chain/info  
node_domain=YOUR_NODE_DOMAIN  
  
## 使用方法

1. M5Stack Core2の電源を入れます。

2. デバイスが自動的にWiFiに接続し、Symbolノードからデータの取得を開始します。

3. 画面にブロック高、最終更新時刻、接続ノードのドメインが表示されます。

4. 更新状態に応じて画面の色が変化します：
- 黒：正常に更新されている
- 黄：30秒以上更新がない
- 赤：5分以上更新がない
