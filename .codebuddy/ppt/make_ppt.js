const pptxgen = require("pptxgenjs");

const NAVY = "16233B";
const NAVY2 = "20344F";
const BG = "F4F6F9";
const CARD = "FFFFFF";
const TEAL = "0E9AA8";
const TEAL_DK = "0B7E89";
const ORANGE = "E8873A";
const TEXT = "22303F";
const MUTED = "63748A";
const LINE = "DCE3EC";
const FONT = "Microsoft YaHei";

const pres = new pptxgen();
pres.layout = "LAYOUT_WIDE"; // 13.333 x 7.5
pres.author = "dev-reload";
pres.title = "HybridCLR 程序集热重载方案分享";

const W = 13.333, H = 7.5;
const mkShadow = () => ({ type: "outer", color: "16233B", blur: 8, offset: 2, angle: 90, opacity: 0.10 });

let pageNo = 0;
function baseSlide(title, kicker) {
  pageNo++;
  const s = pres.addSlide();
  s.background = { color: BG };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: 0.18, h: H, fill: { color: NAVY } });
  if (kicker) {
    s.addText(kicker, { x: 0.55, y: 0.32, w: 11.5, h: 0.32, margin: 0, fontFace: FONT, fontSize: 12, bold: true, color: TEAL_DK, charSpacing: 2 });
  }
  s.addText(title, { x: 0.55, y: 0.62, w: 12.2, h: 0.62, margin: 0, fontFace: FONT, fontSize: 28, bold: true, color: NAVY });
  s.addText(String(pageNo), { x: 12.55, y: 7.02, w: 0.6, h: 0.35, margin: 0, fontFace: FONT, fontSize: 11, color: MUTED, align: "right" });
  return s;
}

function card(s, x, y, w, h, accent) {
  s.addShape(pres.shapes.RECTANGLE, { x, y, w, h, fill: { color: CARD }, line: { color: LINE, width: 1 }, shadow: mkShadow() });
  if (accent) s.addShape(pres.shapes.RECTANGLE, { x, y, w: 0.07, h, fill: { color: accent } });
}

function cardText(s, x, y, w, h, head, body, accent) {
  const runs = [{ text: head, options: { fontSize: 15, bold: true, color: accent || NAVY, breakLine: true } }];
  body.forEach((b, i) => runs.push({ text: b, options: { fontSize: 11.5, color: TEXT, breakLine: i < body.length - 1, bullet: { code: "2022", indent: 10 }, paraSpaceAfter: 4 } }));
  s.addText(runs, { x: x + 0.22, y: y + 0.14, w: w - 0.4, h: h - 0.28, margin: 0, fontFace: FONT, valign: "top" });
}

// ============ S1 封面 ============
{
  const s = pres.addSlide();
  s.background = { color: NAVY };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: W, h: H, fill: { color: NAVY } });
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 5.9, w: W, h: 1.6, fill: { color: NAVY2 } });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.9, y: 1.55, w: 0.14, h: 2.2, fill: { color: TEAL } });
  s.addText("HYBRIDCLR HOT RELOAD", { x: 1.25, y: 1.35, w: 10, h: 0.4, margin: 0, fontFace: FONT, fontSize: 14, bold: true, color: TEAL, charSpacing: 4 });
  s.addText("程序集热重载：Il2CppClass / MethodInfo 指针复用方案", { x: 1.22, y: 1.8, w: 11.9, h: 1.0, margin: 0, fontFace: FONT, fontSize: 30, bold: true, color: "FFFFFF" });
  s.addText("探索历程 · 实现原理 · 踩坑记录 · 根因排查 · 经验总结", { x: 1.25, y: 2.95, w: 10, h: 0.5, margin: 0, fontFace: FONT, fontSize: 17, color: "9FB3C8" });
  s.addText([
    { text: "libil2cpp-hybridclr-reload", options: { bold: true, color: "FFFFFF", breakLine: true } },
    { text: "分支 dev-reload-2022  |  开发周期 2026.07 – 2026.08  |  目标平台 iOS / Android / Windows", options: { color: "9FB3C8" } },
  ], { x: 1.25, y: 6.25, w: 10.5, h: 0.9, margin: 0, fontFace: FONT, fontSize: 12.5 });
}

// ============ S2 探索一：AssemblyLoadContext ============
{
  const s = baseSlide("探索一：AssemblyLoadContext —— 此路不通", "前期尝试 1/3");
  card(s, 0.55, 1.5, 6.0, 4.35, TEAL);
  s.addText([
    { text: "AssemblyLoadContext（ALC）是什么", options: { fontSize: 15, bold: true, color: TEAL_DK, breakLine: true } },
    { text: ".NET Core 3.0+ 引入的程序集隔离加载机制", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "每个 ALC 是独立的加载边界，同一程序集可多版本共存、互不干扰", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "collectible ALC 支持 Unload() 卸载，配合 GC 回收旧程序集", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "AssemblyDependencyResolver 自动解析依赖", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "典型场景：插件系统、动态热更新 —— 看起来正是我们要的", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 0.77, y: 1.66, w: 5.55, h: 4.05, margin: 0, fontFace: FONT, valign: "top" });
  card(s, 6.9, 1.5, 6.0, 4.35, ORANGE);
  s.addText([
    { text: "为什么在 Unity / Il2Cpp 上走不通", options: { fontSize: 15, bold: true, color: ORANGE, breakLine: true } },
    { text: "ALC 属于 System.Runtime.Loader，是 .NET Core/5+ 运行时（CLR）的设施", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "Il2Cpp 是 AOT 编译的裸 C++ 运行时：无 JIT、无真正的程序集加载器", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "Il2CppDomain 的程序集注册全局唯一（s_Assemblies 按名查找，同名即同一个）", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "HybridCLR 解释器程序集也不走 ALC —— 元数据直接进 InterpreterImage", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "结论：ALC 这条路在 Il2Cpp 上根本不存在", options: { fontSize: 11.5, bold: true, color: ORANGE, bullet: { code: "2022", indent: 10 } } },
  ], { x: 7.12, y: 1.66, w: 5.55, h: 4.05, margin: 0, fontFace: FONT, valign: "top" });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 6.1, w: 12.35, h: 0.85, fill: { color: NAVY } });
  s.addText("既然没有现成的隔离加载机制，那就自己实现「同名程序集二次加载」—— 下一站：直接反复加载 Assembly",
    { x: 0.85, y: 6.1, w: 11.8, h: 0.85, margin: 0, fontFace: FONT, fontSize: 13.5, bold: true, color: "FFFFFF", valign: "middle" });
}

// ============ S3 探索二：反复加载 Assembly ============
{
  const s = baseSlide("探索二：让同一个 Assembly 加载第二遍", "前期尝试 2/3");
  card(s, 0.55, 1.5, 6.0, 4.35, ORANGE);
  s.addText([
    { text: "第一关：PlaceHolder 拒绝重载", options: { fontSize: 15, bold: true, color: ORANGE, breakLine: true } },
    { text: "现象：二次加载直接抛异常 —— \"reloading placeholder assembly is not supported!\"", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "根因：占位程序集（PlaceHolder）首次加载后 token != 0，这个标志位被当作「已加载」，重载路径直接拒绝", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "修复：token != 0 时不再抛异常，改走普通新建逻辑，创建全新 Il2CppAssembly / Il2CppImage", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "配套：UnregisterInterpreterAssembly 注销旧程序集 + ReplacePlaceHolderAssembly 让占位列表指向最新注册的同名程序集，支持反复重载", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 0.77, y: 1.66, w: 5.55, h: 4.05, margin: 0, fontFace: FONT, valign: "top" });
  card(s, 6.9, 1.5, 6.0, 4.35, TEAL);
  s.addText([
    { text: "强行加载的结果：一半能跑", options: { fontSize: 15, bold: true, color: TEAL_DK, breakLine: true } },
    { text: "纯 C# 逻辑 OK：普通类、方法调用、新实例化都能正常执行", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "旧程序集有意不释放（存活对象仍可能引用旧类，泄漏换安全）", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "但资源加载一碰就碎：prefab 上的 MonoBehaviour、ScriptableObject 引用全部 Missing", options: { fontSize: 11.5, bold: true, color: ORANGE, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "窗口打不开、资产读不出 —— 对游戏项目来说等于不可用", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 7.12, y: 1.66, w: 5.55, h: 4.05, margin: 0, fontFace: FONT, valign: "top" });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 6.1, w: 12.35, h: 0.85, fill: { color: NAVY } });
  s.addText("Missing 的根源是什么？为什么 Unity 认不得「同一个名字的新类」？—— 追下去，找到了整个方案的地基",
    { x: 0.85, y: 6.1, w: 11.8, h: 0.85, margin: 0, fontFace: FONT, fontSize: 13.5, bold: true, color: "FFFFFF", valign: "middle" });
}

// ============ S4 探索三：Missing 根源 ============
{
  const s = baseSlide("探索三：Missing 的根源 —— Unity 底层缓存", "前期尝试 3/3");
  card(s, 0.55, 1.5, 6.0, 4.35, ORANGE);
  s.addText([
    { text: "Missing 的真正原因", options: { fontSize: 15, bold: true, color: ORANGE, breakLine: true } },
    { text: "Unity 底层大量按「裸指针」缓存脚本类型：", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "MonoManager::m_ScriptImages 启动时按 image 指针注册各程序集", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 14 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "MonoScriptCache / SerializationCache 按 ~klass 指针缓存序列化指令", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 14 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "重载新建 image / 类 → 全是新指针 → Unity 旧引用全部失效 → Missing", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "这正解释了 PlaceHolder Assembly 的设计初衷：启动时占位注册，让 Unity 底层缓存从一开始就指向稳定的 image / 类指针", options: { fontSize: 11.5, bold: true, color: NAVY, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "这些缓存只在 domain unload 时清空，热重载不触发，也没有公开 API 可清", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 0.77, y: 1.66, w: 5.55, h: 4.05, margin: 0, fontFace: FONT, valign: "top" });
  card(s, 6.9, 1.5, 6.0, 4.35, NAVY2);
  s.addText([
    { text: "路线收敛：绕不开，就穿过去", options: { fontSize: 15, bold: true, color: NAVY, breakLine: true } },
    { text: "清空 Unity 缓存：做不到（无 API、不触发 domain unload）", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "让 Unity 认识新指针：等于改 Unity 引擎，不可行", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "唯一出路：保持指针不变 —— 重载时复用旧 Il2CppClass / MethodInfo / image 指针，原地更新其内容指向新元数据", options: { fontSize: 11.5, bold: true, color: TEAL_DK, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "「原地复用」方案由此确立，成为后续一个多月攻坚的主线", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 7.12, y: 1.66, w: 5.55, h: 4.05, margin: 0, fontFace: FONT, valign: "top" });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 6.1, w: 12.35, h: 0.85, fill: { color: NAVY } });
  s.addText("三条路试下来：ALC 不存在、反复加载会 Missing、缓存清不掉 —— 全部指向同一个答案：指针必须稳定",
    { x: 0.85, y: 6.1, w: 11.8, h: 0.85, margin: 0, fontFace: FONT, fontSize: 13.5, bold: true, color: "FFFFFF", valign: "middle" });
}

// ============ S5 为什么必须复用指针 ============
{
  const s = baseSlide("为什么必须「复用指针」", "背景");
  s.addText("热重载 = 不重启进程、不做 domain reload，原地换装程序集。难点不在加载新 DLL，而在旧指针早已「逃逸」到运行时各处。",
    { x: 0.55, y: 1.35, w: 12.2, h: 0.5, margin: 0, fontFace: FONT, fontSize: 13.5, color: TEXT });
  const items = [
    ["对象头 obj->klass", ["每个存活实例的类型 / vtable / 字段与 GC 布局全由它决定", "GC、AOT、解释器按编译期偏移直接解引用，无法拦截"]],
    ["Unity 原生缓存", ["MonoScriptCache / SerializationCache 按 ~klass 指针键控", "仅在 domain unload 时清空 —— 热重载永不触发"]],
    ["托管反射句柄", ["RuntimeType.value / RuntimeMethodInfo.mhandle 存裸指针", "Type == Type 是引用相等，指针变了比较即失败"]],
    ["泛型实例缓存", ["s_GenericClassSet 按 typeHandle 裸指针做 hash", "AOT class_inst->type_argv 直接存 &klass->byval_arg"]],
  ];
  const cw = 6.0, ch = 1.72, gx = 0.55, gy = 2.05, gapX = 0.35, gapY = 0.3;
  items.forEach((it, i) => {
    const x = gx + (i % 2) * (cw + gapX), y = gy + Math.floor(i / 2) * (ch + gapY);
    card(s, x, y, cw, ch, TEAL);
    cardText(s, x, y, cw, ch, it[0], it[1], TEAL_DK);
  });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 6.35, w: 12.35, h: 0.62, fill: { color: NAVY } });
  s.addText("结论：同一逻辑类必须保持同一指针 ——「原地复用、原地更新」是唯一自洽的方案",
    { x: 0.85, y: 6.35, w: 11.8, h: 0.62, margin: 0, fontFace: FONT, fontSize: 14.5, bold: true, color: "FFFFFF", valign: "middle" });
}

// ============ S3 复用架构总览 ============
{
  const s = baseSlide("核心方案：采集 → 三阶段还原", "架构总览");
  const steps = [
    ["1 采集", "CollectReusableObjects\n旧 image 建两张表：\nfullName→旧类 / 签名→旧方法", TEAL_DK],
    ["2 加载", "InitRuntimeMetadatas\n新元数据照常加载\n（复用类不进 _classList）", TEAL_DK],
    ["3 Pass 1", "更新指针\nimage / typeHandle /\nbyval_arg / counts / parent", NAVY2],
    ["4 Rehash", "三个泛型缓存\n先归一化陈旧指针\n再重算 hash 重插", NAVY2],
    ["5 Pass 2", "重置懒加载字段\nmethods/fields/\ntypeHierarchy/init 标志", NAVY2],
    ["6 Pass 3", "Class::Init 重建\n泛型实例三 Pass\n与非泛型交错执行", NAVY2],
  ];
  const sw = 1.95, sh = 2.35, sx = 0.55, sy = 1.6, sgap = 0.13;
  steps.forEach((st, i) => {
    const x = sx + i * (sw + sgap);
    s.addShape(pres.shapes.RECTANGLE, { x, y: sy, w: sw, h: sh, fill: { color: CARD }, line: { color: LINE, width: 1 }, shadow: mkShadow() });
    s.addShape(pres.shapes.RECTANGLE, { x, y: sy, w: sw, h: 0.5, fill: { color: st[2] } });
    s.addText(st[0], { x, y: sy, w: sw, h: 0.5, margin: 0, fontFace: FONT, fontSize: 14, bold: true, color: "FFFFFF", align: "center", valign: "middle" });
    s.addText(st[1], { x: x + 0.1, y: sy + 0.62, w: sw - 0.2, h: sh - 0.75, margin: 0, fontFace: FONT, fontSize: 10.5, color: TEXT, valign: "top" });
    if (i < steps.length - 1) {
      s.addShape(pres.shapes.RIGHT_ARROW, { x: x + sw - 0.02, y: sy + sh / 2 - 0.11, w: 0.18, h: 0.22, fill: { color: TEAL } });
    }
  });
  card(s, 0.55, 4.35, 6.0, 2.5, TEAL);
  cardText(s, 0.55, 4.35, 6.0, 2.5, "方法复用（MethodInfo 同指针）", [
    "签名键：ClassFullName:Method(Params)->Return",
    "TypeToSigString 直接读 TypeDef，不触发类加载",
    "命中则原地更新 token / methodPointer / parameters",
    "interpData 置空，强制重新 Transform 方法体",
  ]);
  card(s, 6.9, 4.35, 6.0, 2.5, ORANGE);
  cardText(s, 6.9, 4.35, 6.0, 2.5, "泛型实例复用", [
    "RestoreCachedGenericClasses 遍历 s_GenericClassSet",
    "AOT 泛型定义的实例：只重置字段，不搬 image",
    "重载后启用 argv 归一化 + GetClass 一次性验证",
    "验证 = 原地指针修补 VAR 参数，不重置任何数组",
  ]);
}

// ============ S4 vtable 混合布局 ============
{
  const s = baseSlide("配套改造：vtable 混合布局", "数据结构");
  s.addText("复用要求新旧类内存布局兼容。原变长 struct 无法保证，改为「固定布局 + 超阈值变长布局」：",
    { x: 0.55, y: 1.35, w: 12.2, h: 0.45, margin: 0, fontFace: FONT, fontSize: 13.5, color: TEXT });
  const stats = [
    ["256", "IL2CPP_MAX_VTABLE_SLOT_COUNT\n内联固定槽数（结构体末尾）"],
    ["224", "阈值：vtable_count > 224\n走变长布局，不再截断"],
    ["+32", "PRESERVED 预留槽\n覆盖数组 interfaceOffset 越界槽"],
  ];
  stats.forEach((st, i) => {
    const x = 0.55 + i * 4.25;
    s.addShape(pres.shapes.RECTANGLE, { x, y: 2.0, w: 3.95, h: 1.85, fill: { color: CARD }, line: { color: LINE, width: 1 }, shadow: mkShadow() });
    s.addText(st[0], { x, y: 2.12, w: 3.95, h: 0.85, margin: 0, fontFace: FONT, fontSize: 40, bold: true, color: TEAL_DK, align: "center" });
    s.addText(st[1], { x: x + 0.2, y: 3.0, w: 3.55, h: 0.75, margin: 0, fontFace: FONT, fontSize: 11, color: MUTED, align: "center" });
  });
  card(s, 0.55, 4.2, 12.35, 2.65, NAVY2);
  s.addText([
    { text: "设计要点", options: { fontSize: 15, bold: true, color: NAVY, breakLine: true } },
    { text: "新增 vtable_allocated_count 字段记录实际分配槽数，所有读写按它做边界", options: { fontSize: 12.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 5 } },
    { text: "复用时 vtable 兼容检查：新 vtable_count ≤ 已分配槽数才允许复用；超出则 MSVC 试 _expand 原地扩容，其他平台放弃复用", options: { fontSize: 12.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 5 } },
    { text: "变长布局写控制台 + vtable_overflow.log（UTC 时间戳，std::gmtime 跨平台单一路径）", options: { fontSize: 12.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 5 } },
    { text: "跨平台约束：禁止仅 Windows 的功能分支，仅允许 #ifdef _MSC_VER 抑制告警", options: { fontSize: 12.5, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 0.8, y: 4.36, w: 11.9, h: 2.35, margin: 0, fontFace: FONT, valign: "top" });
}

// ============ S5 踩坑时间线 ============
{
  const s = baseSlide("踩坑时间线：一个月的攻坚", "历程");
  const tl = [
    ["07-22 前", "方案探索", "ALC 在 Il2Cpp 上不存在；反复加载遇 PlaceHolder 拒绝与 Missing —— 锁定指针复用路线"],
    ["07-23", "复用方案落地", "采集 / 三 Pass 还原 / 方法签名匹配；当天即修 9 个实现 bug（编码索引、parent 置空、忘重新 Init…）"],
    ["07-27", "token / image 错配", "BadImageFormatException、MissingMethodException —— 旧 MethodInfo 的 token 属于旧 image"],
    ["07-28 ~ 30", "泛型地狱", "image 误设 newImage、typeHierarchyDepth 未重置、三缓存 hash 失效 —— 引入 rehash + 指针归一化"],
    ["08-04 ~ 07", "Delegate 绑定失败", "\"method arguments are incompatible\" —— RuntimeType 身份分裂，原地指针修补方案定型"],
    ["08-10 ~ 21", "序列化 NRE（最大 BOSS）", "重载后首开窗口 [SerializeField] 全 null；两周排查，实锤 Unity MonoManager image 缓存"],
    ["08-24", "收官", "image 指针复用 + token 延迟翻转 —— 不崩溃、NRE 消失，全链路验证通过"],
  ];
  const lx = 1.15;
  s.addShape(pres.shapes.LINE, { x: lx + 0.09, y: 1.7, w: 0, h: 5.3, line: { color: LINE, width: 2 } });
  tl.forEach((t, i) => {
    const y = 1.52 + i * 0.78;
    const hot = i === 5 || i === 6;
    s.addShape(pres.shapes.OVAL, { x: lx, y: y + 0.13, w: 0.2, h: 0.2, fill: { color: hot ? ORANGE : TEAL } });
    s.addText(t[0], { x: lx + 0.4, y: y, w: 1.55, h: 0.5, margin: 0, fontFace: FONT, fontSize: 12.5, bold: true, color: hot ? ORANGE : TEAL_DK, valign: "middle" });
    s.addText(t[1], { x: lx + 2.0, y: y, w: 3.1, h: 0.5, margin: 0, fontFace: FONT, fontSize: 13, bold: true, color: NAVY, valign: "middle" });
    s.addText(t[2], { x: lx + 5.2, y: y, w: 6.9, h: 0.75, margin: 0, fontFace: FONT, fontSize: 10.5, color: MUTED, valign: "middle" });
  });
}

// ============ 坑位模板 ============
function pitSlide(kicker, title, phenomenon, rootCause, fix) {
  const s = baseSlide(title, kicker);
  s.addShape(pres.shapes.OVAL, { x: 11.7, y: 0.42, w: 0.95, h: 0.95, fill: { color: ORANGE } });
  s.addText("坑", { x: 11.7, y: 0.42, w: 0.95, h: 0.95, margin: 0, fontFace: FONT, fontSize: 30, bold: true, color: "FFFFFF", align: "center", valign: "middle" });
  const rows = [
    ["现象", phenomenon, ORANGE],
    ["根因", rootCause, NAVY2],
    ["修复", fix, TEAL_DK],
  ];
  let y = 1.5;
  rows.forEach((r) => {
    const lines = r[1];
    const h = 0.55 + lines.length * 0.38;
    card(s, 0.55, y, 12.35, h, r[2]);
    s.addText(r[0], { x: 0.85, y: y + 0.1, w: 1.0, h: 0.4, margin: 0, fontFace: FONT, fontSize: 14, bold: true, color: r[2] });
    const runs = lines.map((t, i) => ({ text: t, options: { fontSize: 12, color: TEXT, breakLine: i < lines.length - 1, bullet: { code: "2022", indent: 10 }, paraSpaceAfter: 3 } }));
    s.addText(runs, { x: 1.95, y: y + 0.12, w: 10.7, h: h - 0.24, margin: 0, fontFace: FONT, valign: "top" });
    y += h + 0.18;
  });
  return s;
}

// ============ S6 坑1 ============
pitSlide("坑 1 / 07-27", "旧 MethodInfo 的 token 属于旧 image",
  [
    "重载后调用旧 delegate / 旧 vtable 方法：BadImageFormatException（方法体解析错乱）",
    "GetMethodInfoFromMethodDef 指针比较失败：MissingMethodException",
  ],
  [
    "复用类的 klass->image 已指向新 image，但旧 MethodInfo 的 token / methodMetadataHandle 仍是旧 image 的",
    "GetMethodBody 在「新 image + 旧 token」上查找 → row index 错位 → 读到错误方法体",
  ],
  [
    "按方法签名匹配复用旧 MethodInfo，原地更新 token / handle / methodPointer 指向新 image",
    "泛型实例：先 Inflate 取新数据再拷贝字段（不能从 methodDefinition 直接拷未 inflate 的值）",
    "interpData 置空强制重新 Transform；s_GenericMethodMap 陈旧 token → ClearStatics 清除",
  ]);

// ============ S7 坑2 ============
pitSlide("坑 2 / 07-28 ~ 30", "泛型缓存：typeHandle 变了，hash 全废",
  [
    "InvalidCastException：EnumEqualityComparer<T> → EqualityComparer<T> 转换失败",
    "GetGenericClass 重复创建实例、ParentMismatch；Expression1<T> → Expression<T> 失败",
  ],
  [
    "Il2CppTypeHash 对 CLASS/VALUETYPE 按 data.typeHandle 裸指针做 hash",
    "Pass 1 更新 typeHandle 后，s_GenericInstSet / 两个 s_GenericClassSet 旧条目 hash 全部失效",
    "AOT 创建的游离 class_inst 不在 s_GenericInstSet，rehash 漏网 → type_argv 半陈旧",
    "klass->image 被误设为当前重载 DLL 的 image（泛型定义在别的 DLL）→ GetMethodBody 越界",
  ],
  [
    "Pass 1 后 rehash 三个缓存：重插前先归一化 type_argv / gclass->type 陈旧指针，再重算 hash",
    "klass->image 只在泛型定义来自解释器时设为 defKlass->image，AOT 定义实例不搬 image",
    "typeHierarchyDepth 必须随 typeHierarchy 一起重置；parent 不能重置（InitLocked 不重建）",
    "泛型还原拆三 Pass 与非泛型交错执行，保证 cctor 访问时继承树已重建",
  ]);

// ============ S8 坑3 ============
pitSlide("坑 3 / 08-04 ~ 07", "Delegate.CreateDelegate：method arguments are incompatible",
  [
    "Action<BackpackTabType, BackpackSubTabType> 绑定失败，异常来自托管 mscorlib 预检",
    "枚举参数只走 Type == Type（RuntimeType 引用相等），不过任何 icall —— 排查盲区",
  ],
  [
    "跨程序集泛型实例（Action`2<程序集A类型, 程序集B类型>）在两个 DLL 分批重载时只归一化了部分 type_argv",
    "methods 以「半陈旧」参数重新 inflate；s_TypeMap 按键内容比较但叶子仍是 typeHandle 裸指针",
    "同一逻辑类型出现两个 RuntimeType 实例 → 引用相等失败（身份分裂）",
  ],
  [
    "重载后启用 argv 归一化：SetupMethods / SetupFields / CreateClass 入口就地归一化",
    "GetClass 挂一次性验证：原地指针修补 VAR 参数（写回 type_argv[num]），不重置任何数组",
    "—— 重置方案会把异步执行链上正在使用的类字段清空，运行时崩溃",
    "配套清理：s_MethodMap 修复、s_ParametersMap / s_TypeMap 清空，消除重复 RuntimeType",
  ]);

// ============ S9 坑4 现象 ============
{
  const s = baseSlide("最大 BOSS：MonoBehaviour 序列化字段 NRE", "坑 4 / 08-10 ~ 21 · 现象");
  s.addShape(pres.shapes.OVAL, { x: 11.7, y: 0.42, w: 0.95, h: 0.95, fill: { color: ORANGE } });
  s.addText("坑", { x: 11.7, y: 0.42, w: 0.95, h: 0.95, margin: 0, fontFace: FONT, fontSize: 30, bold: true, color: "FFFFFF", align: "center", valign: "middle" });
  card(s, 0.55, 1.5, 6.0, 3.1, ORANGE);
  s.addText([
    { text: "现象", options: { fontSize: 15, bold: true, color: ORANGE, breakLine: true } },
    { text: "重载后首次打开的窗口 NRE：bindingSet.Bind(x).For(...) 中 x 为 null", options: { fontSize: 12, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "GetAttrib 返回 null —— [SerializeField] s_currVariable 在内存里真为 0", options: { fontSize: 12, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "运行时字段有值，序列化字段全 null = 反序列化根本没写", options: { fontSize: 12, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 0.8, y: 1.66, w: 5.55, h: 2.8, margin: 0, fontFace: FONT, valign: "top" });
  card(s, 6.9, 1.5, 6.0, 3.1, NAVY2);
  s.addText([
    { text: "A/B 实验（用户）", options: { fontSize: 15, bold: true, color: NAVY, breakLine: true } },
    { text: "场景 A：重载前进过界面（类型预热过）→ 重载后正常", options: { fontSize: 12, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "场景 B：重载前没进过 → 重载后首次进入必 NRE", options: { fontSize: 12, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 4 } },
    { text: "JsonUtility.ToJson 预热（强制构建序列化缓存）可绕过 → 指向 Unity 序列化缓存机制", options: { fontSize: 12, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 7.15, y: 1.66, w: 5.55, h: 2.8, margin: 0, fontFace: FONT, valign: "top" });
  s.addText("两周排除法 —— 全部被证伪：", { x: 0.55, y: 4.9, w: 6, h: 0.4, margin: 0, fontFace: FONT, fontSize: 13.5, bold: true, color: NAVY });
  const ruled = ["vtable 错乱", "方法体错", "字段布局错", "字段类型陈旧", "[SerializeField] 属性丢失", "坏窗口期建空缓存", "重复类 / 身份分裂", "ScriptMissing + Backup 不回填"];
  ruled.forEach((t, i) => {
    const x = 0.55 + (i % 4) * 3.14, y = 5.4 + Math.floor(i / 4) * 0.72;
    s.addShape(pres.shapes.RECTANGLE, { x, y, w: 2.94, h: 0.56, fill: { color: "EDF1F6" }, line: { color: LINE, width: 1 } });
    s.addText([
      { text: "✗ ", options: { color: ORANGE, bold: true } },
      { text: t, options: { color: TEXT } },
    ], { x: x + 0.12, y, w: 2.75, h: 0.56, margin: 0, fontFace: FONT, fontSize: 11.5, valign: "middle" });
  });
}

// ============ S10 坑4 诊断 ============
{
  const s = baseSlide("两周排查靠什么：诊断设施", "坑 4 / 诊断");
  const items = [
    ["ReloadDiagLog", ["header-only，文件 + Unity 日志双写，UTC 时间戳", "重载后才启用 + thread_local 重入保护"]],
    ["NRE 定位", ["解释器 catch 块打印 类::方法 ipOffset token image", "沿异步链传播时每帧一行，首行即抛出点"]],
    ["指令解码 + 写入者回溯", ["从 ipBase 正扫，定位最后写故障槽的指令并解析 callee", "逐 opcode 验证操作数偏移，禁止按指令族猜测"]],
    ["探针矩阵", ["get_fields / field_has_attribute / class_from_name / AllocProbe / gchandle", "TrackedEW：跨时间跟踪托管对象必须 gchandle 保活"]],
  ];
  const cw = 6.0, ch = 1.72;
  items.forEach((it, i) => {
    const x = 0.55 + (i % 2) * (cw + 0.35), y = 1.55 + Math.floor(i / 2) * (ch + 0.3);
    card(s, x, y, cw, ch, TEAL);
    cardText(s, x, y, cw, ch, it[0], it[1], TEAL_DK);
  });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 5.55, w: 12.35, h: 1.35, fill: { color: NAVY } });
  s.addText([
    { text: "血泪教训", options: { fontSize: 13.5, bold: true, color: ORANGE, breakLine: true } },
    { text: "诊断代码在 VM 启动期做类解析会直接崩溃；catch 块里 frame / imi / ip 在帧切换中间态可能互不匹配；通用指令挂钩绝不能盲读 obj->klass（可能是 byref），必须用 imi->method 做过滤前置", options: { fontSize: 12, color: "FFFFFF" } },
  ], { x: 0.85, y: 5.72, w: 11.8, h: 1.05, margin: 0, fontFace: FONT, valign: "top" });
}

// ============ S11 坑4 根因 ============
{
  const s = baseSlide("根因实锤：MonoManager 的 image 缓存不刷新", "坑 4 / 根因");
  const chain = [
    ["重载新建 Il2CppImage", "复用类 klass->image = 新指针"],
    ["MonoManager 不刷新", "m_ScriptImages 启动时经 il2cpp_domain_assembly_open 按指针注册一次"],
    ["GetAssemblyIndexFromImage = -1", "按指针查不到新 image"],
    ["CanTransferTypeAsNestedObject = false", "嵌套 managed 对象字段被序列化指令静默剔除"],
    ["prefab 反序列化跳过字段", "s_currVariable = null → Bind(null) → NRE"],
  ];
  let y = 1.5;
  chain.forEach((c, i) => {
    const bad = i >= 2;
    s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y, w: 8.2, h: 0.82, fill: { color: bad ? "FBEEE0" : CARD }, line: { color: bad ? ORANGE : LINE, width: 1 }, shadow: mkShadow() });
    s.addText([
      { text: c[0] + "   ", options: { fontSize: 13, bold: true, color: bad ? ORANGE : NAVY } },
      { text: c[1], options: { fontSize: 11.5, color: MUTED } },
    ], { x: 0.8, y, w: 7.8, h: 0.82, margin: 0, fontFace: FONT, valign: "middle" });
    if (i < chain.length - 1) s.addShape(pres.shapes.DOWN_ARROW, { x: 4.5, y: y + 0.83, w: 0.22, h: 0.24, fill: { color: TEAL } });
    y += 1.08;
  });
  card(s, 9.1, 1.5, 3.8, 5.3, ORANGE);
  s.addText([
    { text: "决定性证据", options: { fontSize: 15, bold: true, color: ORANGE, breakLine: true } },
    { text: "ImageRegCheck：klass->image ≠ MonoManager 缓存（MATCH=0）", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 5 } },
    { text: "TrackedEW：资产实例与克隆实例的 s_currVariable 均为 null → 资产反序列化就没写", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 }, breakLine: true, paraSpaceAfter: 5 } },
    { text: "不对称现象全解释：子组件指令重载前构建（旧 image 已注册）正常；窗口类指令重载后首次构建（新 image 未注册）失败", options: { fontSize: 11.5, color: TEXT, bullet: { code: "2022", indent: 10 } } },
  ], { x: 9.35, y: 1.66, w: 3.35, h: 5.0, margin: 0, fontFace: FONT, valign: "top" });
}

// ============ S12 坑4 修复 ============
{
  const s = baseSlide("修复：image 指针复用 + token 延迟翻转", "坑 4 / 修复");
  card(s, 0.55, 1.5, 6.0, 2.6, TEAL);
  cardText(s, 0.55, 1.5, 6.0, 2.6, "修复 1：复用旧程序集 / 旧 image 指针", [
    "重载路径 ass = oldAss; image2 = oldAss->image（镜像首载复用占位程序集的做法）",
    "klass->image 恒等于 MonoManager 启动时注册的指针 → 检查通过",
    "只 free image2->name；不 free nameNoExt（raw 字符串堆内指针，free 会堆损坏）",
  ], TEAL_DK);
  card(s, 6.9, 1.5, 6.0, 2.6, TEAL);
  cardText(s, 6.9, 1.5, 6.0, 2.6, "修复 2（关键配套）：token 延迟翻转", [
    "BuildIl2CppImage 不再设 image2->token（保留旧索引）",
    "收集完成后、InitRuntimeMetadatas 之前才翻转为新索引",
    "收集阶段 GetImage 路由到旧 image，收集完路由到新 image —— 太早翻转会导致收集错 image 直接崩溃",
  ], TEAL_DK);
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 4.45, w: 12.35, h: 1.15, fill: { color: "E4F4F1" }, line: { color: TEAL, width: 1 } });
  s.addText([
    { text: "✔ 验证通过（08-24）", options: { fontSize: 14, bold: true, color: TEAL_DK, breakLine: true } },
    { text: "不再崩溃 + NRE 消失 + Delegate 绑定正常；多代重载均成立（token 总是保持上一代索引直到收集完）", options: { fontSize: 12.5, color: TEXT } },
  ], { x: 0.85, y: 4.62, w: 11.8, h: 0.85, margin: 0, fontFace: FONT, valign: "top" });
  s.addText([
    { text: "插曲：第一次 image 复用曾导致启动崩溃并回退", options: { fontSize: 13, bold: true, color: NAVY, breakLine: true } },
    { text: "泛型还原的「双 image 归一化」设计建立在「新旧 image 是不同指针」之上；复用后新旧同指针 → 归一化失效 → typeMetadataHandle 越界。token 延迟翻转正是为兼容这套设计。", options: { fontSize: 12, color: MUTED } },
  ], { x: 0.55, y: 5.85, w: 12.35, h: 1.1, margin: 0, fontFace: FONT, valign: "top" });
}

// ============ S13 Adapter 评估 ============
{
  const s = baseSlide("方案权衡：为什么不用 Adapter 替代复用", "方案评估");
  s.addText("提议：il2cpp API 出方向返回 Adapter 壳、入方向拆包，重载只换壳内 real 指针，从而抛弃复用。",
    { x: 0.55, y: 1.32, w: 12.2, h: 0.42, margin: 0, fontFace: FONT, fontSize: 12.5, color: TEXT });
  card(s, 0.55, 1.85, 6.0, 2.9, TEAL);
  cardText(s, 0.55, 1.85, 6.0, 2.9, "边界层本身可行（≈ 10–20%）", [
    "real ↔ adapter 双射表可覆盖 Unity 引擎边界",
    "image 指针稳定 = 已实施的 image 复用修复",
    "Il2CppType 经 TypeAdapter→ClassAdapter→realKlass 间接链可保鲜",
  ], TEAL_DK);
  card(s, 6.9, 1.85, 6.0, 2.9, ORANGE);
  cardText(s, 6.9, 1.85, 6.0, 2.9, "三条内部边界包不住（≈ 80–90%）", [
    "obj->klass：GC / AOT / 解释器编译期偏移直解引用，放不了壳",
    "托管反射句柄：RuntimeType.value 裸指针，icall 读写不经 API",
    "泛型缓存 + AOT 游离 class_inst：typeHandle 裸指针 hash，枚举不全",
  ], ORANGE);
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 5.0, w: 12.35, h: 0.9, fill: { color: NAVY } });
  s.addText([
    { text: "账本：删掉内部复用 → 三条内部边界全崩；保留复用 → Adapter 冗余。", options: { fontSize: 13, bold: true, color: "FFFFFF", breakLine: true } },
    { text: "减负方向是降低复用实现复杂度（swap bodies / 工程化沉淀不变式），而非绕开复用。", options: { fontSize: 12, color: "9FB3C8" } },
  ], { x: 0.85, y: 5.12, w: 11.8, h: 0.7, margin: 0, fontFace: FONT, valign: "top" });
  s.addText("佐证：Unity 侧 ScriptingTypePtr::operator== 穿透到 il2cpp 内容比较，叶子仍落 typeHandle 裸指针 ——「指针身份唯一」约束贯穿 Unity 层 / API 层 / 托管层。",
    { x: 0.55, y: 6.1, w: 12.35, h: 0.8, margin: 0, fontFace: FONT, fontSize: 11.5, color: MUTED, italic: true, valign: "top" });
}

// ============ S14 热重载友好设计 ============
{
  const s = baseSlide("另一条路：热重载友好的架构设计", "换个思路");
  s.addText("与其在运行时底层硬扛指针复用，不如从架构上让「需要被 Unity 序列化的类型」根本不参与热更 —— 类似 Lua 集成的通用 LuaBehaviour 模式。",
    { x: 0.55, y: 1.32, w: 12.3, h: 0.55, margin: 0, fontFace: FONT, fontSize: 13, color: TEXT });
  const steps = [
    ["通用壳类型", "固定的通用 MonoBehaviour / ScriptableObject，放在非热更程序集（AOT），永不重载", TEAL_DK],
    ["序列化归它", "资源 prefab / ScriptObject 的序列化与加载全由壳类型承担 —— Unity 缓存的指针永远稳定", TEAL_DK],
    ["转发逻辑", "壳类型持有类型名 / 方法名，经标准 Assembly.GetType 从热更程序集解析真实类型，把生命周期回调转发过去", NAVY2],
    ["热更侧瘦身", "热更程序集只剩纯逻辑类，不继承 MonoBehaviour —— 回到探索二的「二次加载」即可，无需复用 Il2CppClass", NAVY2],
  ];
  const cw = 3.0, ch = 2.5, gx = 0.55, gy = 2.05, gap = 0.15;
  steps.forEach((st, i) => {
    const x = gx + i * (cw + gap);
    s.addShape(pres.shapes.RECTANGLE, { x, y: gy, w: cw, h: ch, fill: { color: CARD }, line: { color: LINE, width: 1 }, shadow: mkShadow() });
    s.addShape(pres.shapes.RECTANGLE, { x, y: gy, w: cw, h: 0.52, fill: { color: st[2] } });
    s.addText(`${i + 1}  ${st[0]}`, { x: x + 0.12, y: gy, w: cw - 0.24, h: 0.52, margin: 0, fontFace: FONT, fontSize: 13.5, bold: true, color: "FFFFFF", valign: "middle" });
    s.addText(st[1], { x: x + 0.14, y: gy + 0.66, w: cw - 0.28, h: ch - 0.8, margin: 0, fontFace: FONT, fontSize: 11, color: TEXT, valign: "top" });
    if (i < steps.length - 1) {
      s.addShape(pres.shapes.RIGHT_ARROW, { x: x + cw - 0.015, y: gy + ch / 2 - 0.11, w: 0.18, h: 0.22, fill: { color: TEAL } });
    }
  });
  card(s, 0.55, 4.85, 6.0, 2.0, TEAL);
  cardText(s, 0.55, 4.85, 6.0, 2.0, "收益", [
    "避开复用全套复杂度：无需 Pass 1-3 / rehash / 指针修补",
    "改写代码量很少，业务侧只加一层壳类型转发",
    "不碰 libil2cpp 内部结构，Unity 版本升级几乎零成本",
  ], TEAL_DK);
  card(s, 6.9, 4.85, 6.0, 2.0, ORANGE);
  cardText(s, 6.9, 4.85, 6.0, 2.0, "代价与前提", [
    "Inspector 可视化 / 序列化字段定义移到壳类型，热更侧字段改动需同步壳",
    "每次回调多一跳转发 + GetType 解析（可缓存）",
    "适合新项目或愿意改造工作流；存量项目迁移成本高",
  ], ORANGE);
}

// ============ S15 工程使用方法 ============
{
  const s = baseSlide("工程使用方法：三步接入", "如何使用");
  const steps = [
    ["Install", "Unity 编辑器中使用正常的 HybridCLR 菜单执行 Install", "HybridCLR/Installer 菜单安装后，会在项目下生成改造过的 il2cpp 库目录：\nHybridCLRData\\\nLocalIl2CppData-{Runtime}\\", TEAL_DK],
    ["替换", "把该目录下的 il2cpp 库内容替换成本 git 库内容即可", "本仓库 = HybridCLR 官方 libil2cpp + 热重载改造（指针复用 / 二次加载 / vtable 混合布局）\n替换后重新构建 GameAssembly 生效", NAVY2],
    ["使用", "业务侧照常走 HybridCLR 热更流程加载 DLL", "同一程序集二次加载即触发热重载路径：复用旧类 / 旧方法指针，原地接上新元数据", TEAL_DK],
  ];
  const cw = 3.95, ch = 3.6, gx = 0.55, gy = 1.6, gap = 0.25;
  steps.forEach((st, i) => {
    const x = gx + i * (cw + gap);
    s.addShape(pres.shapes.RECTANGLE, { x, y: gy, w: cw, h: ch, fill: { color: CARD }, line: { color: LINE, width: 1 }, shadow: mkShadow() });
    s.addShape(pres.shapes.RECTANGLE, { x, y: gy, w: cw, h: 0.58, fill: { color: st[3] } });
    s.addText(`STEP ${i + 1}   ${st[0]}`, { x: x + 0.15, y: gy, w: cw - 0.3, h: 0.58, margin: 0, fontFace: FONT, fontSize: 14.5, bold: true, color: "FFFFFF", valign: "middle" });
    s.addText([
      { text: st[1], options: { fontSize: 12.5, bold: true, color: NAVY, breakLine: true, paraSpaceAfter: 8 } },
      { text: st[2], options: { fontSize: 11, color: MUTED } },
    ], { x: x + 0.18, y: gy + 0.72, w: cw - 0.36, h: ch - 0.9, margin: 0, fontFace: FONT, valign: "top" });
    if (i < steps.length - 1) {
      s.addShape(pres.shapes.RIGHT_ARROW, { x: x + cw + 0.03, y: gy + ch / 2 - 0.12, w: 0.2, h: 0.24, fill: { color: TEAL } });
    }
  });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.55, y: 5.55, w: 12.35, h: 1.3, fill: { color: NAVY } });
  s.addText([
    { text: "为什么这么简单？", options: { fontSize: 13.5, bold: true, color: TEAL, breakLine: true } },
    { text: "全部改造都在 libil2cpp 层完成 —— 对 Unity 引擎与 HybridCLR 上层完全透明，不依赖任何 Unity 版本特定补丁；升级 Unity 时只需对新版 LocalIl2CppData 重新做一次替换。", options: { fontSize: 12, color: "FFFFFF" } },
  ], { x: 0.85, y: 5.72, w: 11.8, h: 1.0, margin: 0, fontFace: FONT, valign: "top" });
}

// ============ S16 经验总结 ============
{
  const s = baseSlide("经验总结", "Takeaways");
  const lessons = [
    ["指针即身份", "Unity 层 / API 层 / 托管层三层一致约束，只有复用能同时满足；同型两指针必然身份分裂"],
    ["缓存即雷区", "凡按裸指针 hash / 键控的缓存（泛型实例、TypeMap、MethodMap），重载后必须 rehash 或失效"],
    ["懒惰重建门各不相同", "methods / properties / events 按指针门；fields 按 size_inited 标志门 —— 置 NULL 必须连标志清"],
    ["修复代码三重安全", "重载后启用的代码必须：启动期安全 + 递归安全 + restore Pass 期间静默（守卫计数器）"],
    ["诊断先行", "双写日志 + 重入保护 + 逐 opcode 验证；让 bug 自己说话，比读代码猜根因快一个数量级"],
    ["跨平台纪律", "标准 C++ 单一路径（std::gmtime 等），禁止仅 Windows 的功能分支，仅允许告警抑制 pragma"],
  ];
  lessons.forEach((l, i) => {
    const x = 0.55 + (i % 2) * 6.35, y = 1.5 + Math.floor(i / 2) * 1.78;
    card(s, x, y, 6.0, 1.6, null);
    s.addShape(pres.shapes.OVAL, { x: x + 0.18, y: y + 0.18, w: 0.52, h: 0.52, fill: { color: i < 3 ? TEAL : NAVY2 } });
    s.addText(String(i + 1), { x: x + 0.18, y: y + 0.18, w: 0.52, h: 0.52, margin: 0, fontFace: FONT, fontSize: 16, bold: true, color: "FFFFFF", align: "center", valign: "middle" });
    s.addText([
      { text: l[0], options: { fontSize: 14, bold: true, color: NAVY, breakLine: true } },
      { text: l[1], options: { fontSize: 11, color: MUTED } },
    ], { x: x + 0.88, y: y + 0.14, w: 5.0, h: 1.35, margin: 0, fontFace: FONT, valign: "top" });
  });
}

// ============ S15 结尾 ============
{
  const s = pres.addSlide();
  s.background = { color: NAVY };
  s.addShape(pres.shapes.RECTANGLE, { x: 0, y: 0, w: W, h: H, fill: { color: NAVY } });
  s.addShape(pres.shapes.RECTANGLE, { x: 0.9, y: 2.6, w: 0.14, h: 1.7, fill: { color: TEAL } });
  s.addText("热重载的复杂度，是在为运行时内部一致性付账", { x: 1.25, y: 2.55, w: 11, h: 0.8, margin: 0, fontFace: FONT, fontSize: 28, bold: true, color: "FFFFFF" });
  s.addText("复用不是目的，让三层指针身份唯一才是。", { x: 1.25, y: 3.45, w: 11, h: 0.5, margin: 0, fontFace: FONT, fontSize: 16, color: "9FB3C8" });
  s.addText("Q & A", { x: 1.25, y: 4.6, w: 6, h: 0.6, margin: 0, fontFace: FONT, fontSize: 20, bold: true, color: TEAL });
}

pres.writeFile({ fileName: "热重载方案分享.pptx" }).then(() => console.log("OK"));

