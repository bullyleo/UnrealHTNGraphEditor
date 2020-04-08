# HTN Graph Plugin(EN)

Editor for building HTN domains on Unreal Engine 4 (UE4).

## Required plugin

- **HTN Planner**

## Supported features

- Editor for creating HTN Domain

## Unsupported features

- Debug
  - Blakepoint
  - Highlighting running nodes

## How to use

- Select ** HTNGraphEditor-> HTN Graph Editor ** from Add New button of Content Browser

<img src="assets/2020-01-18_11h16_04.png" height=350>

- Open HTN Graph Asset and build Domain

<img src="assets/2020-01-18_11h27_08.png" height=350>

- Add ** HTN Component ** (this is an Actor Component) to the Blueprint that inherits from the Pawn or Character class

<img src="assets/2020-01-18_11h28_20.png">

- Set the Enum asset representing the World State to ** World State Enum Asset ** in the details panel of HTN Component and set ** Default World State **

<img src="assets/2020-01-18_11h47_05.png" height=350>

- To execute an action based on HTN planning, set an Enum asset representing the operation to be executed in ** Operator Enum Asset ** in the details panel of HTN Component, and set ** HTNO Operator Classes ** to ** HTNOperator_BlueprintBase ** Register a Blueprint that inherits the class.

<img src="assets/2020-01-18_11h47_06.png" height=350>

- Pull the pin from the HTN Component in the ** Begin Play ** of the Blueprint to which the HTN Component is attached. Call the ** Build HTN Domain ** and ** Generate Plan **. At this time, HTN Graph used by AI is specified in Build HTN Domain.

![2020-01-18_11h49_51](assets/2020-01-18_11h49_51.png)

- With the above, the action plan to be executed by AI can be obtained from HTN Graph Editor

---

- If the action to be executed is specified, the operator registered in HTN Operator Classes by calling ** Execute Operator ** from HTN Component using ** Tick event ** or ** Set Timer By Event ** Is executed based on the action plan.

![2020-01-18_11h51_39](assets/2020-01-18_11h51_39.png)



## How to add a plugin to another project

- After creating another project, create a ** Plugins folder ** at the same level as the Content folder and Config folder

![2020-01-18_18h06_42](assets/2020-01-18_18h06_42.png)

- Copy and paste the contents of the ** PackagePlugin folder ** into the created Plugins folder

![2020-01-18_18h07_51](assets/2020-01-18_18h07_51.png)

---

# プロジェクトについて

Unreal Engine 4（UE4）でHTNドメインを構築するためのエディタを提供する。

## 必須プラグイン

- **HTN Planner**

## サポートしている機能

- HTN Domain作成のためのエディタ

## サポートしていない機能

- デバッグ機能
  - ブレークポイントの配置
  - 実行中のノードのハイライト

## 使い方

- Content BrowserのAdd Newボタンから **HTNGraphEditor -> HTN Graph Editor** を選択

<img src="assets/2020-01-18_11h16_04.png" height=350>



- HTN Graph Editorを開きDomainを構築する

<img src="assets/2020-01-18_11h27_08.png" height=350>

- PawnもしくはCharacterクラスを継承したブループリントに **HTN Component** （これはActor Component）を追加する

<img src="assets/2020-01-18_11h28_20.png">

- HTN Component の詳細パネルにある **World State Enum Asset** にWorld Stateを表すEnumアセットをセットし**Default World State** を設定する

<img src="assets/2020-01-18_11h47_05.png" height=350>

- HTNプランニングに基づいたアクションを実行させる場合にはHTN Componentの詳細パネルにある **Operator Enum Asset** に実行するオペレーションを表すEnumアセットをセットし **HTN Operator Classes** に**HTNOperator_BlueprintBase** クラスを継承したブループリントを登録する。

<img src="assets/2020-01-18_11h47_06.png" height=350>

- HTN Componentがアタッチされたブループリントの **Begin Play** にてHTN Componentからピンを引っ張り **Build HTN Domain** と **Generate Plan** を呼ぶ。この時Build HTN DomainにはAIが使用するHTN Graphを指定する。

![2020-01-18_11h49_51](assets/2020-01-18_11h49_51.png)

- 以上でHTN Graph EditorからAIが実行するべきアクションプランが得られる

---

- もし実行するアクションを指定している場合は **Tickイベント** もしくは **Set Timer By Event** を利用してHTN Component から **Execute Operator** を呼ぶことでHTN Operator Classesに登録したオペレータをアクションプランを基に実行する。

![2020-01-18_11h51_39](assets/2020-01-18_11h51_39.png)



## 他プロジェクトへプラグインを追加するには

- 他プロジェクト作成後、ContentフォルダやConfigフォルダと同一階層に **Pluginsフォルダ** を作成する
![2020-01-18_18h06_42](assets/2020-01-18_18h06_42.png)

- 作成したPluginsフォルダに **PackagePluginフォルダの中身を** コピー＆ペースト

![2020-01-18_18h07_51](assets/2020-01-18_18h07_51.png)

- 念の為プロジェクトを開いて **HTNGraphプラグインとHTNPlannerプラグイン** が有効化されているかを確認する。