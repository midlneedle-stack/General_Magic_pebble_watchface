'use client';

import {
  FormEvent,
  ReactNode,
  startTransition,
  useCallback,
  useEffect,
  useLayoutEffect,
  useMemo,
  useRef,
  useState,
  useId,
} from "react";

type HourlyStrength = "light" | "medium" | "hard";

type Settings = {
  timeFormat: "12" | "24";
  theme: "light" | "dark";
  vibration: boolean;
  animation: boolean;
  vibrateOnOpen: boolean;
  hourlyChime: boolean;
  hourlyChimeStrength: HourlyStrength;
};

const HOURLY_STRENGTHS: HourlyStrength[] = ["light", "medium", "hard"];

const DEFAULT_SETTINGS: Settings = {
  timeFormat: "24",
  theme: "dark",
  vibration: true,
  animation: true,
  vibrateOnOpen: true,
  hourlyChime: false,
  hourlyChimeStrength: "medium",
};

const normalizeStrength = (value: unknown): HourlyStrength => {
  if (typeof value === "string") {
    const normalized = value.toLowerCase();
    if (HOURLY_STRENGTHS.includes(normalized as HourlyStrength)) {
      return normalized as HourlyStrength;
    }
  }
  return "medium";
};

const parseIncomingState = (raw: unknown): Partial<Settings> => {
  if (!raw || typeof raw !== "object") {
    return {};
  }
  const data = raw as Record<string, unknown>;
  const next: Partial<Settings> = {};

  if (data.timeFormat === "12" || data.timeFormat === "24") {
    next.timeFormat = data.timeFormat;
  }
  if (data.theme === "light" || data.theme === "dark") {
    next.theme = data.theme;
  }
  if (typeof data.vibration === "boolean") {
    next.vibration = data.vibration;
  }
  if (typeof data.animation === "boolean") {
    next.animation = data.animation;
  }
  if (typeof data.vibrateOnOpen === "boolean") {
    next.vibrateOnOpen = data.vibrateOnOpen;
  }
  if (typeof data.hourlyChime === "boolean") {
    next.hourlyChime = data.hourlyChime;
  }
  if (typeof data.hourlyChimeStrength !== "undefined") {
    next.hourlyChimeStrength = normalizeStrength(
      data.hourlyChimeStrength,
    );
  }

  return next;
};

const readStateFromQuery = (): Partial<Settings> => {
  if (typeof window === "undefined") {
    return {};
  }
  const params = new URLSearchParams(window.location.search);
  const raw = params.get("state");
  if (!raw) {
    return {};
  }
  try {
    return parseIncomingState(JSON.parse(raw));
  } catch {
    try {
      return parseIncomingState(JSON.parse(decodeURIComponent(raw)));
    } catch (err) {
      console.warn("general-magic-config: failed to parse state", err);
      return {};
    }
  }
};

export default function Home() {
  const [settings, setSettings] = useState<Settings>(DEFAULT_SETTINGS);

  useEffect(() => {
    const incoming = readStateFromQuery();
    if (Object.keys(incoming).length > 0) {
      startTransition(() => {
        setSettings((prev) => ({ ...prev, ...incoming }));
      });
    }
  }, []);

  const updateSetting = <K extends keyof Settings>(
    key: K,
    value: Settings[K],
  ) => {
    setSettings((prev) => ({ ...prev, [key]: value }));
  };

  const handleSubmit = (event: FormEvent<HTMLFormElement>) => {
    event.preventDefault();
    const payload = encodeURIComponent(JSON.stringify(settings));
    if (typeof window !== "undefined") {
      window.location.href = `pebblejs://close#${payload}`;
    }
  };

  const handleReset = () => {
    setSettings({ ...DEFAULT_SETTINGS });
  };

  const handleAnimationToggle = (value: boolean) => {
    setSettings((prev) => ({
      ...prev,
      animation: value,
      vibrateOnOpen: value ? prev.vibrateOnOpen : false,
    }));
  };

  const handleHourlyStrengthChange = (value: HourlyStrength) => {
    setSettings((prev) => ({
      ...prev,
      hourlyChime: true,
      hourlyChimeStrength: value,
    }));
  };

  const handleHourlyOn = () => {
    setSettings((prev) => ({
      ...prev,
      hourlyChime: true,
    }));
  };

  const handleHourlyOff = () => {
    setSettings((prev) => ({
      ...prev,
      hourlyChime: false,
    }));
  };

  const payloadPreview = useMemo(
    () => JSON.stringify(settings, null, 2),
    [settings],
  );

  return (
    <div className="min-h-screen bg-slate-100 px-4 py-10 text-slate-900">
      <div className="mx-auto flex w-full max-w-2xl flex-col gap-6">
        <header className="rounded-2xl border border-slate-200 bg-white p-6 shadow-sm">
          <h1 className="text-2xl font-semibold text-slate-900">
            General Magic Settings
          </h1>
          <p className="mt-2 text-sm text-slate-500">
            Minimal configuration page for the watchface. Adjust everything
            you need and send the data straight back to the Pebble app.
          </p>
        </header>

        <form
          onSubmit={handleSubmit}
          className="space-y-6 rounded-2xl border border-slate-200 bg-white p-6 shadow-sm"
        >
          <Field label="Time format">
            <select
              value={settings.timeFormat}
              onChange={(event) =>
                updateSetting(
                  "timeFormat",
                  event.target.value === "12" ? "12" : "24",
                )
              }
              className="w-full rounded-lg border border-slate-300 bg-white px-3 py-2 text-sm outline-none focus:border-slate-500"
            >
              <option value="24">24-hour</option>
              <option value="12">12-hour</option>
            </select>
          </Field>

          <Field label="Theme">
            <select
              value={settings.theme}
              onChange={(event) =>
                updateSetting(
                  "theme",
                  event.target.value === "light" ? "light" : "dark",
                )
              }
              className="w-full rounded-lg border border-slate-300 bg-white px-3 py-2 text-sm outline-none focus:border-slate-500"
            >
              <option value="dark">Dark</option>
              <option value="light">Light</option>
            </select>
          </Field>

          <CheckboxField
            label="Vibration"
            helper="Enable global vibration feedback."
            checked={settings.vibration}
            onChange={(checked) => updateSetting("vibration", checked)}
          />

          <CheckboxField
            label="Animation"
            helper="Smooth screen transitions on launch."
            checked={settings.animation}
            onChange={handleAnimationToggle}
          />

          <CheckboxField
            label="Vibrate when opening the watchface"
            helper="Requires animation to be enabled."
            checked={settings.vibrateOnOpen}
            disabled={!settings.animation}
            onChange={(checked) => updateSetting("vibrateOnOpen", checked)}
          />

          <HourlyChimeControl
            enabled={settings.hourlyChime}
            strength={settings.hourlyChimeStrength}
            onStrengthChange={handleHourlyStrengthChange}
            onTurnOn={handleHourlyOn}
            onTurnOff={handleHourlyOff}
          />

          <div className="flex flex-wrap justify-end gap-3 pt-4">
            <button
              type="button"
              onClick={handleReset}
              className="rounded-xl border border-slate-300 px-4 py-2 text-sm font-semibold text-slate-700 transition hover:bg-slate-100"
            >
              Reset
            </button>
            <button
              type="submit"
              className="rounded-xl bg-slate-900 px-4 py-2 text-sm font-semibold text-white transition hover:bg-slate-800"
            >
              Send to watch
            </button>
          </div>

          <pre className="rounded-2xl bg-slate-900/90 p-4 text-xs text-white">
            {payloadPreview}
          </pre>
        </form>
      </div>
    </div>
  );
}

type HourlyChimeControlProps = {
  enabled: boolean;
  strength: HourlyStrength;
  onStrengthChange: (value: HourlyStrength) => void;
  onTurnOn: () => void;
  onTurnOff: () => void;
};

type ChimeAnchor = HourlyStrength | "on";

const CHIME_LABEL: Record<ChimeAnchor, string> = {
  on: "On",
  light: "Light",
  medium: "Medium",
  hard: "Hard",
};

function HourlyChimeControl({
  enabled,
  strength,
  onStrengthChange,
  onTurnOn,
  onTurnOff,
}: HourlyChimeControlProps) {
  const trackRef = useRef<HTMLDivElement | null>(null);
  const anchorRefs = useRef<Record<ChimeAnchor, HTMLButtonElement | null>>({
    on: null,
    light: null,
    medium: null,
    hard: null,
  });

  const activeAnchor: ChimeAnchor = enabled ? strength : "on";
  const [pillStyle, setPillStyle] = useState<{ width: number; x: number }>({
    width: 0,
    x: 0,
  });
  const [pillLabel, setPillLabel] = useState(CHIME_LABEL[activeAnchor]);
  const [pillLabelVisible, setPillLabelVisible] = useState(true);

  const recalcPill = useCallback(() => {
    const track = trackRef.current;
    const target = anchorRefs.current[activeAnchor];
    if (!track || !target) return;
    const trackRect = track.getBoundingClientRect();
    const targetRect = target.getBoundingClientRect();

    setPillStyle({
      width: targetRect.width,
      x: targetRect.left - trackRect.left,
    });
  }, [activeAnchor]);

  useLayoutEffect(() => {
    recalcPill();
  }, [recalcPill]);

  useEffect(() => {
    const handleResize = () => recalcPill();
    window.addEventListener("resize", handleResize);
    const id = window.setTimeout(recalcPill, 60);
    return () => {
      window.removeEventListener("resize", handleResize);
      window.clearTimeout(id);
    };
  }, [recalcPill]);

  useEffect(() => {
    setPillLabelVisible(false);
    const fadeTimeout = window.setTimeout(() => {
      setPillLabel(CHIME_LABEL[activeAnchor]);
      setPillLabelVisible(true);
    }, 90);
    return () => window.clearTimeout(fadeTimeout);
  }, [activeAnchor]);

  const handleStrengthSelect = (value: HourlyStrength) => {
    if (!enabled) {
      onTurnOn();
    }
    onStrengthChange(value);
  };

  const handlePillActivate = () => {
    if (!enabled) {
      onTurnOn();
    }
  };

  const pillEasing = "cubic-bezier(0.18, 0.89, 0.35, 1.15)";

  return (
    <div className="space-y-4 rounded-2xl border border-slate-200 bg-slate-50/60 p-4">
      <div className="flex items-start justify-between gap-4">
        <div className="space-y-1">
          <p className="text-sm font-semibold text-slate-900">Hourly chime</p>
          <p className="text-xs text-slate-500">
            {enabled
              ? "Pick how strong the hourly ping should feel."
              : "Tap on to let the button morph into a strength."}
          </p>
        </div>
        <button
          type="button"
          onClick={enabled ? onTurnOff : onTurnOn}
          className={`rounded-full px-3 py-1.5 text-xs font-semibold transition-colors ${
            enabled
              ? "border border-slate-300 text-slate-700 hover:border-slate-400 hover:text-slate-900"
              : "bg-slate-900 text-white shadow-[0_8px_20px_rgba(15,23,42,0.25)] hover:bg-slate-800"
          }`}
        >
          {enabled ? "Off" : "On"}
        </button>
      </div>

      <div className="relative isolate overflow-hidden rounded-xl border border-slate-200 bg-white px-3 py-4 shadow-[0_10px_40px_rgba(15,23,42,0.05)]">
        <div
          ref={trackRef}
          className="relative grid grid-cols-3 gap-2 text-sm font-semibold text-slate-500"
        >
          <div
            onClick={handlePillActivate}
            role={!enabled ? "button" : undefined}
            tabIndex={!enabled ? 0 : -1}
            onKeyDown={(event) => {
              if (!enabled && (event.key === "Enter" || event.key === " ")) {
                event.preventDefault();
                handlePillActivate();
              }
            }}
            className={`absolute left-0 top-1/2 h-11 -translate-y-1/2 rounded-lg bg-slate-900 text-white shadow-[0_14px_34px_rgba(15,23,42,0.20)] transition-[transform,width,box-shadow] ${
              enabled ? "pointer-events-none" : "cursor-pointer"
            }`}
            style={{
              width: pillStyle.width ? pillStyle.width : undefined,
              transform: `translateX(${pillStyle.x}px)`,
              transitionDuration: "280ms",
              transitionTimingFunction: pillEasing,
              boxShadow: enabled
                ? "0 16px 44px rgba(15, 23, 42, 0.22)"
                : "0 12px 28px rgba(15, 23, 42, 0.16)",
            }}
          >
            <div className="flex h-full items-center justify-center px-4">
              <span
                className="text-sm transition-opacity duration-200"
                style={{ opacity: pillLabelVisible ? 1 : 0 }}
              >
                {pillLabel}
              </span>
            </div>
          </div>

          <button
            ref={(node) => (anchorRefs.current.light = node)}
            type="button"
            onClick={() => handleStrengthSelect("light")}
            className={`relative z-10 rounded-lg px-3 py-2 transition-colors duration-200 ${
              enabled ? "text-slate-700 hover:text-slate-900" : "text-slate-400"
            }`}
          >
            Light
          </button>
          <button
            ref={(node) => (anchorRefs.current.medium = node)}
            type="button"
            onClick={() => handleStrengthSelect("medium")}
            className={`relative z-10 rounded-lg px-3 py-2 transition-colors duration-200 ${
              enabled ? "text-slate-700 hover:text-slate-900" : "text-slate-400"
            }`}
          >
            Medium
          </button>
          <button
            ref={(node) => (anchorRefs.current.hard = node)}
            type="button"
            onClick={() => handleStrengthSelect("hard")}
            className={`relative z-10 rounded-lg px-3 py-2 transition-colors duration-200 ${
              enabled ? "text-slate-700 hover:text-slate-900" : "text-slate-400"
            }`}
          >
            Hard
          </button>

          <button
            ref={(node) => (anchorRefs.current.on = node)}
            type="button"
            aria-hidden
            tabIndex={-1}
            className="pointer-events-none absolute left-1/2 top-1/2 h-11 w-24 -translate-x-1/2 -translate-y-1/2 opacity-0"
          >
            On
          </button>
        </div>
      </div>
    </div>
  );
}

type FieldProps = {
  label: string;
  helper?: string;
  children: ReactNode;
};

function Field({ label, helper, children }: FieldProps) {
  return (
    <div className="flex flex-col gap-2">
      <span className="text-xs font-semibold uppercase tracking-wide text-slate-500">
        {label}
      </span>
      {children}
      {helper ? (
        <p className="text-xs text-slate-500">{helper}</p>
      ) : null}
    </div>
  );
}

type CheckboxFieldProps = {
  label: string;
  helper?: string;
  checked: boolean;
  disabled?: boolean;
  onChange: (checked: boolean) => void;
};

function CheckboxField({
  label,
  helper,
  checked,
  disabled,
  onChange,
}: CheckboxFieldProps) {
  const checkboxId = useId();

  return (
    <div
      className={`flex items-start justify-between gap-4 rounded-2xl border border-slate-200 bg-slate-50/80 px-4 py-3 ${disabled ? "opacity-60" : ""}`}
    >
      <div className="pr-3">
        <label
          htmlFor={checkboxId}
          className="text-sm font-medium text-slate-900"
        >
          {label}
        </label>
        {helper ? (
          <p className="mt-1 text-xs text-slate-500">{helper}</p>
        ) : null}
      </div>
      <input
        id={checkboxId}
        type="checkbox"
        checked={checked}
        disabled={disabled}
        onChange={(event) => onChange(event.target.checked)}
        className="mt-1 h-5 w-5 rounded border border-slate-400 accent-slate-900"
      />
    </div>
  );
}
