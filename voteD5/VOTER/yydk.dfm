object yydkform: Tyydkform
  Left = 343
  Top = 205
  Width = 260
  Height = 170
  Caption = '应到人数'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 16
    Top = 16
    Width = 128
    Height = 16
    Caption = '请输入应到人数：'
    Font.Charset = GB2312_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = '宋体'
    Font.Style = []
    ParentFont = False
  end
  object Edit1: TEdit
    Left = 40
    Top = 48
    Width = 57
    Height = 21
    TabOrder = 0
    Text = '0'
  end
  object UpDown1: TUpDown
    Left = 97
    Top = 48
    Width = 17
    Height = 21
    Associate = Edit1
    Min = 0
    Position = 0
    TabOrder = 1
    Wrap = False
  end
  object Button1: TButton
    Left = 32
    Top = 96
    Width = 75
    Height = 25
    Caption = '确定'
    Font.Charset = GB2312_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = '宋体'
    Font.Style = []
    ModalResult = 1
    ParentFont = False
    TabOrder = 2
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 144
    Top = 96
    Width = 75
    Height = 25
    Caption = '取消'
    Font.Charset = GB2312_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = '宋体'
    Font.Style = []
    ModalResult = 2
    ParentFont = False
    TabOrder = 3
  end
end
