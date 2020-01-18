# プロジェクトについて

Unreal Engine 4（UE4）でHTNドメインを構築するためのエディタを提供する。

## 必須プラグイン

- **HTN Planner**

## 使い方

- Content BrowserのAdd Newボタンから **HTNGraphEditor -> HTN Graph Editor** を選択

![2020-01-18_11h16_04](assets/2020-01-18_11h16_04.png)

- HTN Graph Editorを開きDomainを構築する

![2020-01-18_11h27_08](assets/2020-01-18_11h27_08.png)

- PawnもしくはCharacterクラスを継承したブループリントに **HTN Component** （これはActor Component）を追加する

![2020-01-18_11h28_20](assets/2020-01-18_11h28_20.png)

- HTN Component の詳細パネルにある **World State Enum Asset** にWorld Stateを表すEnumアセットをセットし**Default World State** を設定する

![2020-01-18_11h47_05](assets/2020-01-18_11h47_05.png)

- HTNプランニングに基づいたアクションを実行させる場合にはHTN Componentの詳細パネルにある **Operator Enum Asset** に実行するオペレーションを表すEnumアセットをセットし **HTN Operator Classes** に**HTNOperator_BlueprintBase** クラスを継承したブループリントを登録する。

![2020-01-18_11h47_06](assets/2020-01-18_11h47_06.png)

- HTN Componentがアタッチされたブループリントの **Begin Play** にてHTN Componentからピンを引っ張り **Build HTN Domain** と **Generate Plan** を呼ぶ。この時Build HTN DomainにはAIが使用するHTN Graphを指定する。

![2020-01-18_11h49_51](assets/2020-01-18_11h49_51.png)

- 以上でHTN Graph EditorからAIが実行するべきアクションプランが得られる

---

- もし実行するアクションを指定している場合は **Tickイベント** もしくは **Set Timer By Event** を利用してHTN Component から **Execute Operator** を呼ぶことでHTN Operator Classesに登録したオペレータをアクションプランを基に実行する。

![2020-01-18_11h51_39](assets/2020-01-18_11h51_39.png)



## 他プロジェクトへプラグインを追加するには

- Pluginsフォルダをそのまま他のプロジェクトフォルダにコピー＆ペースト
- プロジェクトフォルダに追加後プロジェクトを開き **HTN Graphプラグイン** と **HTN Plannerプラグイン**の両方を有効化する。



# メモ

ライセンスは「MITライセンス」で提供。

