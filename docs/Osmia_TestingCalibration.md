# Testing and calibration of the ALMaSS *Osmia bicornis* model


**Intended publication:** extensive appendix to the MIDox paper for this model

**Authors:** Xiaodong Duan, Elżbieta Ziółkowska, Christopher John Topping



---

## Abstract

Agent-based models of solitary bees are increasingly used in environmental risk assessment, but their parameters are rarely observable in isolation and are typically assembled from separate literature sources. We report the verification, calibration and structural evaluation of the ALMaSS *Osmia bicornis* model against its published Formal Model, using a hierarchy of five field-observed patterns and a common error metric. Extracting the temperature-driven development and emergence code as a separable sub-model — an exact rather than approximate reduction, validated against the compiled semantics by unit tests — reduced a full eleven-parameter search from days to seconds.

The a priori parameterisation published in the Formal Model does not sustain the population. Under weather series from France, Poland and Germany it predicts that no cohort completes in-nest development before winter, at sites where the species is demonstrably present; the pupal stage runs 298 days against a field target of 36. Relaxing each parameter individually identifies the pupal base temperature as the single value responsible: it is the only change that restores persistence, from 0% to 94% and 100% survival. The larval base temperature, by contrast, is confirmed at its published value.

Constraining every developmental base temperature to successively higher floors and refitting shows why. Fit degrades continuously from nRMSE 0.039 unconstrained to 0.130 at a 10 °C floor, then collapses between 10 and 12 °C — a boundary beyond which the Formal Model's egg (13.8 °C) and pupal (13.2 °C) values both lie. Base temperature and degree-day requirement are two coordinates on one curve, jointly identified by the data and not separately sourceable. We report two further instances of the same failure: two independently sourced mass conversions that disagree by 12.4% on the same 601 individuals, and nest-density parameters attributed to a source that supplies only a relative index. We also show that the fitting can purchase apparent persistence by collapsing the emergence distribution to a single day, and that this degeneracy is produced by the threshold constraint rather than incidental to it.

The calibrated parameterisation reproduces the field stage durations at both sites to within a few per cent (nRMSE 0.039) and places emergence onset at DOY 103, inside an independent window not used in fitting. We report explicitly which results carry quantified uncertainty and which do not, and which model domains remain speculative. Scope is verification, calibration, validation and structural evaluation; a global sensitivity and uncertainty analysis is deferred until the calibration protocol is complete.

We conclude with a procedural recommendation: a Formal Model should record which of its parameters are jointly identified, distinguish measured from assumed values, and be run once under realistic forcing before publication. Each inconsistency reported here would have been caught by that, at a cost of hours.

## Keywords

Agent-based model; ALMaSS; *Osmia bicornis*; pattern-oriented modelling; model verification; model calibration; degree-day development; parameter identifiability; solitary bee

---

## 1. Introduction

### 1.1 Where this paper sits

Model documentation in this framework proceeds through three publications (Topping et al. 2022): the **Formal Model**, which states modelling intent and reviews the data foundation before implementation; the **MIDox paper**, which documents the implementation itself; and the **Testing and Calibration paper**, which evaluates whether the implementation behaves as intended and estimates the parameters it needs. The Formal Model for *Osmia bicornis* was published by Ziółkowska et al. (2023). This paper is the third component, and appears as an appendix to the MIDox paper for the same model.

The separation matters here in a specific way. Testing is not the terminal step of a linear pipeline; it feeds back into implementation understanding, and this paper is a worked instance of that loop. Verification found a defect in the prepupal development routine (Section 3.3), the source was amended, and every result reported here was produced against the amended code. That is why a paper nominally about calibration spends its third section on implementation defects: the defects were found by the testing process and could not have been found any other way.

### 1.2 Why calibrating a solitary-bee model is hard

Calibrating a population model of a solitary bee is constrained by the animal's biology in ways that are worth stating, because they determine the method.

*Osmia bicornis* is univoltine. Each individual passes through the observable in-nest stages once, in one year, and the adult female is on the wing for a few weeks. A study therefore yields one observation per cohort per site, and the quantities that matter most — how long each stage takes under real weather — cannot be observed as a time series in the way that a multivoltine insect or a vertebrate with repeated censuses permits. The available field records are correspondingly few: for this model, two, from different countries and different years.

Those records are also heterogeneous in a way that resists pooling. Stage durations, emergence dates, cells per nest and adult mass come from different studies, in different countries, under thermal conditions that are usually unrecorded and sometimes only qualitative. The laboratory measurements that do carry precise temperatures were made at constant temperatures the animal never experiences, and a model fitted to constant-temperature data does not transfer to a fluctuating field regime — a point this paper demonstrates quantitatively (Sections 4.14 and Table A3) rather than assumes.

The deeper difficulty is that the parameters of interest are not independently observable. A degree-day model's base temperature and its degree-day requirement are two coordinates on one curve: many pairs reproduce the same durations, and no experiment identifies either alone. The same is true of the multiplier from cocoon mass to provision mass and the slope from provision mass to adult mass, which jointly determine one measurable relationship. Sourcing such parameters individually from separate studies — the natural procedure when assembling a model from the literature — can therefore produce a pair that is internally incoherent, and the incoherence is invisible until the model runs under realistic forcing. This paper reports three independent instances (Section 6.3).

Pattern-oriented modelling (Grimm et al. 2005) is the appropriate response because it does not ask any single dataset to identify any single parameter. It asks the model to reproduce several weak, independently observed patterns simultaneously, and it accepts a parameterisation only if it is defensible against each. Where individual patterns are uninformative — and several of ours are, taken alone — their conjunction is not.

One result should be signposted here because it is unusual. **The published a priori parameterisation for this model does not work.** It is a careful, peer-reviewed synthesis of the literature, and under all three European weather series tested it predicts that no cohort survives its first winter, at sites where the species is demonstrably present. Section 4.6 reports the failure, Section 4.7 identifies the single parameter responsible, and Section 6.3 argues that the failure mode is general rather than particular to this model.

### 1.3 Calibration begins from the Formal Model

Calibration here begins from the Formal Model. That parameterisation is the model's a priori specification: assembled from the published literature, documented, and peer-reviewed independently of any simulation result. During development the operational code accumulated a number of revised parameter values, arrived at informally in response to problems encountered during testing and recorded only as trailing comments in the source. Those values were fitted against data, but the procedure was neither systematic nor documented, and the resulting set was never evaluated against the full set of patterns under a common metric. They are therefore intermediate working state rather than a scientific alternative to the Formal Model, and this paper supersedes them. They are not reported as a comparator; where the implementation departed structurally from the specification, that is documented in Section 3.3.

### 1.4 Scope

This paper reports verification, calibration against a five-level pattern hierarchy, validation against held-out data, and structural evaluation. It does **not** report a formal global sensitivity analysis or a variance-based uncertainty analysis.

Two reasons. First, the calibration is incomplete: Stages 3, 5 and 6 of the protocol are unfitted (Section 4.15), so a global analysis would be run over a parameter set that is going to change. Second, the Stage 1–2 results have not yet been cross-checked against the full model (Section 5.4), and a sensitivity analysis inherits every uncertainty of the parameterisation it is run around.

We do report a **one-factor constrained-refit analysis** of the developmental base temperatures (Section 4.8), because that parameter class proved decisive for model behaviour and its response is the paper's principal quantitative result. A global sensitivity and uncertainty analysis is deferred until the full protocol is complete.

Dropping the formal uncertainty analysis does not dispose of the editorial's requirement to report parameter sets with uncertainty quantification. It obliges us instead to state plainly what uncertainty we have quantified and what we have not, which is done in Section 7.

---

## 2. The model

### 2.1 Purpose and scope

The model represents *Osmia bicornis* as a mechanistic, individual-based system in which development, behaviour and reproduction emerge from interactions with weather, floral resources and spatial landscape structure. Built within the ALMaSS framework (Topping et al. 2003; Topping 2022), it links empirically grounded biological processes to a dynamic, management-driven environment. Core drivers include temperature-dependent brood development and emergence timing, nest-site selection, time- and season-limited foraging and provisioning, and brood-cell construction with resource-dependent sex allocation, allowing population-level outcomes to arise from individual behavioural rules.

The intended model is specified in full by Ziółkowska et al. (2023). The implementation — landscape and vegetation dynamics, floral resource production, dispersal and short-range movement, nest-site selection, foraging, provisioning, sex allocation, mortality and parasitism — is documented in the accompanying MIDox paper and is not repeated here. Only the parts of the model on which the results below directly depend are described, in Section 2.2.

### 2.2 In-nest development, emergence, and the day-0 cull

Development proceeds egg → larva → prepupa → pupa → cocooned adult. Three distinct mechanisms are used, and the distinction matters for everything that follows.

**Egg, larva and pupa** accumulate degree-days above a stage-specific lower developmental threshold towards a stage-specific sum of effective temperatures (`Osmia_Egg::st_Develop`, `Osmia_Larva::st_Develop`, `Osmia_Pupa::st_Develop`). Sub-threshold days consume time but contribute no degree-days, and negative degree-days do not subtract. Stage transitions use a strict `>` comparison, so the day on which the threshold is crossed counts towards the stage duration.

**The prepupa** instead uses a non-linear temperature-response function, reflecting the observation that prepupal development is fastest at intermediate temperatures (Bosch et al. 2008; Sgolastra et al. 2012). The Formal Model specifies prepupal duration as "a quadratic function (a mean of the functions presented in Fig. 3C) with an optimum at 22 °C, at which the maximal developmental speed of 24.3 days is reached", with "an individual variation … around a maximal developmental speed (+/− 10%)" (Ziółkowska et al. 2023, p.12). Daily development rate is therefore *r*(*T*) = *q*(*T*opt)/*q*(*T*), normalised to 1.0 at the optimum; coefficients are given in Appendix A, Table A1b.

**The cocooned adult** is governed by a three-phase state machine keyed to both temperature and calendar date (`Osmia_InCocoon::st_Develop`). Pre-wintering accumulates degree-days above 15 °C, which determine overwintering mortality rather than development, and ends when the population manager detects a sustained autumn temperature drop. Overwintering then accumulates degree-days until 1 March, at which point an emergence counter is set from the accumulated total plus a draw from the emergence kernel and a per-nest aspect delay. The counter decrements only on days at or above the emergence temperature threshold, and emergence occurs when it falls below one.

**The day-0 cull.** `Osmia_Population_Manager::DoBefore()` terminates every surviving larva, prepupa and pupa on day 0 of each year, imposing a hard 31 December deadline on completing in-nest development. Cocooned adults and adult females are exempt. The cull therefore applies to exactly the three stages that use temperature-driven somatic development and exempts the stage in which the species overwinters. It is a biological statement, not an implementation convenience: an individual that has not reached the cocooned-adult stage by the onset of winter is dead. This was confirmed with the model authors, and its consequences are taken up in Section 6.2. The persistence constraint it generates is therefore admissible as evidence, and is used as such throughout Section 4.

---

## 3. Verification

Verification asks whether the code correctly implements the intended algorithms. It is logically prior to calibration: fitting parameters to a defective implementation estimates the wrong quantity. This section reports what was checked, what was found, and — equally important under the editorial's framing — what could not be checked.

### 3.1 Sub-model extraction and the equivalence argument

Stages 1 and 2 were fitted outside the full agent-based model. The in-nest development and emergence code reads only the daily temperature supply and static configuration values; it never accesses the landscape, polygons, pollen maps, forage masks, or any other agent. Extracting it into a standalone driver is therefore not an approximation but the same computation on the same inputs, and it reduces a full parameter search from days to seconds. The one coupling that leaks in is the per-nest uniform 0–15 day aspect offset applied at emergence, which is drawn independently of everything else and is therefore sampled rather than simulated.

The extraction is the enabling step for everything in Section 4. It is also the single largest threat to the validity of those results, which is why it is reported here as a verification object rather than buried in the methods, and why Section 5.4 treats the outstanding full-model cross-check as a limitation on the standing of the results rather than as future work.

### 3.2 Unit testing against C++ semantics

The extraction was validated against C++ semantics with a dedicated test suite (`test_osmia_phenology.py`) covering the details most easily lost in translation:

- strict `>` stage transitions, with the crossing day counted towards the stage;
- sub-threshold days consuming time without contributing degree-days;
- non-subtracting negative degree-days;
- the pre-decrement test on the emergence counter (`--counter < 1`), which determines whether the emergence-kernel draw can act at all (Section 4.9).

Each of these is a place where a natural Python transcription differs from the C++ by one day or one unit, and each would have been absorbed silently into a fitted parameter.

### 3.3 Defects found in the implementation

Three departures from the specification were identified. All are structural rather than parametric, and all were resolved before calibration.

**The prepupal rate function carried a flat additive term (resolved).** *The source has been amended and now evaluates the specified quadratic directly; all results in this paper were produced with the corrected implementation. The departure is documented here because it materially affected the model's thermal response and because earlier testing was conducted under it.*

The specified quadratic had been implemented as a 42-entry lookup table indexed on temperature rounded to the nearest degree. That table reproduced the specified function faithfully: fitting a quadratic to the reciprocal of the array recovers the coefficients with a maximum absolute residual of 1.9 × 10⁻⁸, and reconstructing the array as *q*(22)/*q*(*T*) returns every published entry to within 9 × 10⁻¹⁰. The tabulation was therefore not itself a departure, only a discretisation.

The departure was in `Osmia_Prepupa::st_Develop`, which evaluated the stage-completion test as `if (m_AgeDegrees++ > m_myOsmiaPrepupaDevelTotalDays)`. The post-increment added a flat 1.0 per day on top of the temperature-dependent rate. Because the rate function is normalised to a maximum of 1.0, that flat term was of the same order as the entire temperature signal. It did not merely shorten the stage; it compressed the thermal response:

| Temperature | As specified | As implemented |
|---|---|---|
| 10 °C | 79.6 d | 34.5 d |
| 15 °C | 43.9 d | 29.0 d |
| 22 °C | 24.3 d | 22.5 d |
| 30 °C | 45.5 d | 29.3 d |

The 10 °C-to-22 °C duration ratio was 3.28 as specified against 1.53 as implemented, so the prepupa was roughly half as temperature-sensitive as intended and markedly too fast in cool conditions — which is where the northern-European weather series sit.

The correction removed the post-increment and replaced the lookup table with continuous evaluation of the quadratic from four configurable coefficients (Appendix A, Table A1b). Note that the two changes are not independent: `OSMIA_PREPUPADEVELDAYS` had been set to 45 in the presence of the flat term, where the effective peak rate was 2.0 rather than 1.0, and so encoded approximately twice the intended target. Removing the term without also restoring the parameter would have left the stage roughly twice as long as specified. The default is now the Formal Model value of 24.3 d.

**An undocumented bimodality in planned egg number.** `Osmia_Female::PlanEggsPerNest()` contains `if (g_rand_uni_fnc() > 0.55) shift = 2;`, adding two eggs to 45% of nests. This has no counterpart in the Formal Model and no stated empirical basis. We have not removed it, because doing so would change reproductive output by an amount we cannot presently justify in either direction, and because Pattern 3 is provisional for independent reasons (Section 4.12). It is reported as an open question for the model authors, and its effect is currently absorbed into the fitted BETA shape rather than separately identified.

A second undocumented behaviour in the same routine has the same status: `PlanEggsPerNest()` is called once per female rather than once per nest, so all between-nest variation within a female's lifetime derives from a single draw made at emergence, modified only by the deterministic −2 ramp. Whether that is intended is likewise unresolved.

**A latent out-of-bounds read.** The tabulated prepupal lookup clamped its index below at 0 but not above at 41, so a daily mean temperature above 41.5 °C would have indexed past the end of a 42-element vector. Not reachable in the weather series used here, but removed by the change above.

### 3.4 Internal-consistency checks

Two checks compare quantities the model computes by more than one route. Neither is a comparison with data; both are tests of whether the implementation is self-consistent, and both failed as configured.

**Two routes to provision mass disagree by 12.4%.** The model converts cocoon mass to provision mass by a fixed multiplier, and separately converts provision mass to adult mass by a linear relation. Applied to the same 601 females, the two routes give mean provision masses of 397.8 mg and 353.9 mg — a difference of 43.9 mg (+12.4%, SD 19.1), with a correlation of 0.967 between them. This is a systematic scale-and-offset discrepancy, not noise. Recalibration (Section 4.11) closes it exactly.

**Two routes to lifetime egg number disagree by 16%.** The lifetime budget in `CalculateEggLoad()` uses Seidelmann et al.'s (2010) mass regression multiplied by `OSMIA_TOTALNESTSPOSSIBLE`; the realised nest sequence uses `PlanEggsPerNest()` followed by a −2 ramp for each subsequent nest. Nothing in the code requires them to agree, so their agreement is a testable consistency condition. Under the configuration default (`OSMIA_EGGSPERNESTPROBARGS = 1.0 4.00`) the two routes give 26.4 against 31.4 eggs, a 15.9% deficit; under the value recorded in the code comment (`1.8 5`) they agree to 0.1%. This is evidence that the comment records the intended value and the configuration default is the error, rather than the reverse (Section 4.12).

Consistency checks of this kind are cheap and, in this model, productive. Both defects were silent: neither produced a crash, an obviously wrong output, or a pattern mismatch that behavioural testing would have caught.

### 3.5 An interaction between the calibrated mass mapping and the viability check

`Osmia_Female::Init` rejects any female whose mass falls outside
[`OSMIA_MINFEMALEMASS`, `OSMIA_MAXFEMALEMASS`] with `std::exit()`. Adult mass is *α* + *β* × provision,
so the minimum provision that clears the floor is (`MINFEMALEMASS` − *α*)/*β*, and the calibrated pair
raises it substantially:

| Parameterisation | *β* | *α* | Provision needed for a 25 mg female |
|:---|---:|---:|---:|
| Formal Model | 0.2500 | 4.00 | 84.0 mg |
| Calibrated (Section 4.11) | 0.2324 | 0.00 | **107.6 mg** |

The calibrated pair is correct as a description of the cocoon-to-adult relationship — the intercept is
not distinguishable from zero, and a zero-mass cocoon yielding a 4 mg adult is not interpretable —
but it raises the provisioning effort required for a viable daughter by 28%. That is a real
consequence of the calibration and is discussed in Section 4.11.

**The implementation is self-consistent, so the check does not fire.** Both routes by which a female
can be created derive their mass bounds from the *same* configured *α* and *β* that the check uses,
rather than assuming the Formal Model values:

- `Osmia_Base::SetOsmiaParameters` computes
  `m_FemaleMinTargetProvisionMass = (m_FemaleMinMass − α)/β`, and
  `Osmia_Female::LayEgg` lays a female cell only where the accumulated provision exceeds that target,
  switching the cell to male otherwise. A female cell therefore cannot be closed below the mass floor.
- The initial cohort in `Osmia_Population_Manager` draws starting provision masses uniformly on
  [(`MINFEMALEMASS` − *α*)/*β*, (`MAXFEMALEMASS` − *α*)/*β*], so the founding females are in range
  by construction.

Whatever mapping is configured, a female reaching `Init` clears the floor. Earlier drafts of this
section reported a hard failure; that was correct for an implementation that derived the provisioning
target independently of the mapping, and it no longer applies.

Two observations survive the fix. First, the `std::exit()` remains a blunt response to a condition
that is now, by construction, a programming error rather than a parameter choice — a diagnostic abort
is defensible there, but it means any future change that decouples the provisioning target from the
mass mapping will reintroduce a hard crash rather than a degradation. Second, the derived floor is
exact only up to floating-point rounding: a founding female drawn at precisely the lower bound maps
back to 25.0 mg with no margin.

### 3.6 Input and data hygiene

Three input problems were fixed before production runs. `Phenology_testing.xlsx` was not a valid archive and was recovered from a conflict copy. Weather file column order was inconsistent between sites (`year month day` for two files, `day month year` for a third), a silent-failure risk that would have produced nonsense dates rather than a crash; column order is now detected from the data rather than assumed. Finally, `weather_Regensburg_DE_1995_2010.pre` was confirmed to contain 2000–2009, not 1995–2010 as its filename claims; it does cover the 2008 study year.

All target data were extracted programmatically from the project workbooks into version-controlled CSV files, each row carrying a `source_cell` reference to its originating sheet and cell (2,498 records across ten files). No target value is entered by hand.

### 3.7 What could not be verified

The editorial names mass-balance checks, conservation-property verification and comparison with analytical solutions. None applies here, and saying so explicitly is more useful than silence.

The model conserves no quantity: individuals are created by oviposition and removed by mortality, and pollen is consumed from a landscape that regenerates on its own schedule. There is no analytical solution to compare against, because the quantity of interest — the distribution of stage durations under a real weather series — has no closed form. The nearest available check is the degree-day arithmetic itself, which is verified by the unit tests in Section 3.2.

The substantive verification gap is **integration**: the extracted sub-model has not been run against the full ALMaSS implementation. This is the check that would confirm the equivalence argument of Section 3.1 empirically rather than by inspection, and it has not been performed. Its absence is the principal limitation on the standing of the Section 4 results (Section 5.4).

---

## 4. Calibration

### 4.1 The pattern hierarchy

We follow pattern-oriented modelling (Grimm et al. 2005): the model must reproduce multiple independent patterns simultaneously, using a parameter set defensible against each pattern individually. A model that fits one pattern well and others poorly is weaker than one that fits all moderately.

| Level | Pattern | Observable | Data | What it constrains | Why it is informative | Fit measure | Status |
|:---|:---|:---|:---|:---|:---|:---|:---|
| Individual, sub-adult | **1 — In-nest stage durations** | Days per stage under field weather | Poznań 2000; Regensburg 2008; 7 laboratory records 17.5–30 °C | 7 development parameters | The only pattern constraining thermal response under fluctuating temperature. Laboratory records constrain response *shape*, field records constrain totals | nRMSE, per-site block | **Fitted**, 0.039 |
| Individual, adult | **2 — Emergence onset** | DOY of first emergence | Lusignan 1971; Rothamsted | 4 overwintering and emergence parameters | Integrates the whole preceding thermal history, so a stage-duration error propagates into onset | nRMSE on onset | **Fitted** (onset only) |
| Individual, adult | **2b — Emergence spread** | SD or range of emergence | none usable | emergence kernel width | Would break the degeneracy of Section 4.9 | — | **Gap** |
| Nest | **3 — Cells per nest** | Distribution across 352 nests | Ivanov (2006) | BETA shape, `TOTALNESTSPOSSIBLE` | Constrains reproductive allocation; the consistency requirement between the two egg-number routes identifies `TOTALNESTSPOSSIBLE`, which the distribution alone cannot | Wasserstein, 0.329 | **Provisional** |
| Individual, mass | **4 — Female emergence mass** | Distribution of adult mass | 597 field records; 1,305 paired cocoon/adult | mass-from-provision mapping | The *mapping* is identifiable by regression without simulation; the *distribution* is not | *R*² 0.936, slope CI | **Mapping fitted; distribution not** |
| Population | **5 — Persistence** | Fraction of cohorts reaching the cocooned-adult stage before winter | The species is present and persistent at all three sites | all of the above | It is what falsifies the a priori parameterisation | % survival | **Used as a filter; multi-year stability outstanding** |

Two features of this hierarchy should be stated rather than left to inference.

**Persistence is a pattern, not a numerical constraint.** Because the day-0 cull encodes a biological fact (Section 2.2), the survival column encodes a field observation: *O. bicornis* is present and persistent in Poland, Germany and France, so any parameterisation predicting that no cohort survives its first winter at those sites is falsified by the field. This is a *weak* pattern in Grimm's sense — almost trivially true, uninformative on its own — and it is nonetheless the pattern doing the most work in this paper, because it is what rejects the a priori parameterisation. That is precisely the pattern-oriented argument for hierarchies of weak patterns.

One precision is required, because a reader will otherwise draw the wrong inference from the survival column. The relationship is **asymmetric**: 0% survival falsifies a parameterisation, but 100% survival does not validate one. It only fails to reject it. Survival is a filter, not a fit target, and the "100 / 100" in Table A2 is not a measure of quality.

**The hierarchy has a hole at the nest-to-population transition.** Stages 3 and 5 are unfitted (Section 4.15), so the chain from individual development to population dynamics is not yet closed. This is a real incompleteness and is reported as such.

### 4.2 Target data and provenance

#### 4.2.1 Pattern 1 — In-nest stage durations

Two field records under natural conditions provide the primary targets:

| Stage | Poznań, 2000 (Giejdasz & Wilkaniec 2002) | Regensburg, 2008 (Radmacher & Strohm 2011) |
|---|---|---|
| Egg | 7.6 d | 7.0 d |
| Larva | 39.1 d (♀) | 34 d |
| Prepupa | — | 25 d |
| Pupa | — | 36 d |
| Prepupa + pupa | 54.2 d | 61 d |
| Total | 93.3 d | 102 d |

A further seven laboratory records span 17.5–30 °C at constant temperature. These constrain the *shape* of the temperature response; the field records constrain total degree-days.

Three caveats are carried in the extracted data and handled explicitly rather than silently. Field temperatures are recorded only as "14 (average diurnal)" for Poznań and as unknown for Regensburg, so neither field record supports degree-day fitting without reconstructing temperature from the weather series. The egg column is not sex-resolved in the source and is marked as pooled. Giejdasz & Wilkaniec's 28 °C pupal values (12.4 ♂, 10.9 ♀) are flagged as outliers in the source workbook's own base-temperature fit and are excluded explicitly in the fitting code.

The two field datasets are mutually inconsistent and there is no basis for preferring one over the other, so the fitted model balances errors approximately equally between them (Section 4.13).

One bookkeeping discrepancy should be recorded. The Formal Model's Table 2 gives the Giejdasz & Wilkaniec female total as 100.9 d, the sum of the component stages (7.6 + 39.1 + 54.2). The project workbook gives 93.3 d, exactly 7.6 d lower, because its total column excludes the egg stage; the same offset appears in the male row (95.5 summed against 87.9 tabulated). The extracted targets carry the workbook convention. This does not affect the fit, since composite stages are excluded from the error metric, but the two conventions must not be mixed when totals are reported.

#### 4.2.2 Pattern 2 — Adult emergence phenology

Lusignan 1971 (Tasei & Picart 1973) provides a full seasonal activity curve, DOY 104–174, 11 observations of relative activity. Rothamsted (Raw 1972) provides median female activity dates of DOY 144 (1968) and 142 (1969), and Kehrberger & Holzschuh (2019) provide emergence start and end windows for Würzburg, Germany, in 2015 (DOY 100–106 and 124–135 respectively, recorded as ranges rather than point estimates).

Only **onset** is fitted. The Tasei & Picart series counts active females through the season, so its spread confounds emergence timing with adult lifespan and detectability; fitting the spread or the weighted mean would fit the wrong quantity. Model onset is taken as the 1st percentile of the sampled emergence distribution rather than its minimum, since the minimum over thousands of draws is an unstable extreme-value statistic, and the earliest observed record carries 0.12% of seasonal activity. The Kehrberger & Holzschuh window is held out of the fitting entirely and used for validation (Section 5.1).

The emergence variation kernel (`OSMIA_EMERGENCEPROBARGS`) deserves note. It is the April 1st cohort of Giejdasz & Wasilewski, a single overwintering treatment rather than pooled data, and it is already the widest of the six available cohorts. Widening it further to address an over-rapid emergence therefore means extrapolating beyond the dataset, not selecting a different cohort.

#### 4.2.3 Pattern 3 — Eggs per nest

The Ivanov (2006) distribution, 21 records, nest counts totalling 352. Both planned and achieved distributions are reported, since the gap between them is diagnostic (Section 4.12). Note that this is *cells per nest*, not eggs per female, and the source workbook flags that the model treats single-egg nests differently from the field definition; the mapping must be confirmed before these values are adopted.

#### 4.2.4 Pattern 4 — Female emergence mass

A field distribution of 597 adult female body-mass records (range 24.4–147.7 mg, mean 92.63, SD 17.36), supported by 1,305 paired body-to-cocoon mass records and 180 sex-ratio records, all A. Bednarska's Wielkopolska field data. The 601 female records in the paired file are the same individuals as the 597 plus four with cocoon mass but no usable adult mass; the two are therefore not independent lines of evidence. The sex-ratio records are derived from fitted equations rather than observed, and are treated as a prior specification to be reproduced rather than as validation data.

Pattern 4 differs from the others in one important respect: its core parameters are identifiable **without simulation**. The model maps cocoon mass to provision mass by a fixed multiplier and provision mass to adult mass linearly, so composing gives a single claim about two directly observed quantities:

  adultMass = (`FEMALEMASSFROMPROVMASSSLOPE` × `OSMIAS_PROVISIONINGTOCOCOON`) × cocoonMass + `FEMALEMASSFROMPROVMASSCONST`

The 1,305 paired records test that composite by regression, with no ALMaSS run and no assumed forage input. Note the identifiability limit: only the **product** is identified from paired cocoon and adult masses. Since Seidelmann (2006) measured provision-to-cocoon directly, we hold that multiplier fixed and attribute any discrepancy to the slope and intercept.

Female mass determines the bee size score, which feeds planned eggs per nest, which determines provisioning demand, which determines daughter mass. Patterns 3 and 4 are coupled through this feedback loop within the full model; the regression breaks the loop by conditioning on observed cocoon mass rather than simulating it.

### 4.3 Which parameters are calibrated, and which are held fixed

| Class | Parameters | Treatment | Reason |
|:---|:---|:---|:---|
| In-nest development | egg, larva, pupa thresholds and SETs; prepupal scale | **Fitted** (7) | Not independently observable under field forcing; jointly identified (Section 7.1) |
| Overwintering and emergence | overwintering threshold, emergence threshold, counter constant and slope | **Fitted** (4) | As above; not separable from the in-nest stages (Section 4.5) |
| Prepupal rate function | *a*, *b*, *c*, *T*opt | **Fixed** | Specified by the Formal Model; only the scale is free |
| Prepupal individual variation | ±10% | **Fixed** | Assumed in the Formal Model; no data to fit against |
| Emergence kernel | `EMERGENCEPROBARGS` | **Fixed** | An empirical distribution, already the widest available cohort |
| Cocoon-to-provision multiplier | `PROVISIONINGTOCOCOON` | **Fixed** | Measured directly by Seidelmann (2006); only its product with the slope is identified |
| Mass-from-provision mapping | slope, intercept | **Fitted by regression** | Identifiable without simulation |
| Eggs per nest | BETA shape, `TOTALNESTSPOSSIBLE` | **Fitted, provisional** | Jointly identified by a consistency constraint |
| Foraging, movement, mortality, parasitism, pesticide | — | **Not fitted** | Stages 3, 5 and 6 outstanding |

Eleven parameters are free in the Stage 1–2 fit. This is a deliberate restriction: full joint optimisation over approximately twenty free parameters against four pattern classes would be under-determined and would produce compensating errors.

### 4.4 Error metric

A single metric is defined once and used throughout, so that stages, sites and hypotheses remain comparable.

For Stages 1 and 2 the metric comprises three equally weighted blocks: Poznań stage durations, Regensburg stage durations, and Lusignan emergence onset. Within a duration block, errors are normalised by the target and combined as a root mean square across the directly observed stages, so a 10% miss on the egg stage counts the same as a 10% miss on the pupa. Composite stages (prepupa + pupa, total) are reported but excluded from the error so they are not double-counted. A stage that never completes takes the maximum penalty.

For the distributional targets of Patterns 3 and 4 we use a distributional distance — Wasserstein — rather than comparing means, since earlier testing found the model matched mean emergence timing while badly mismatching spread.

Per-block error is reported alongside the aggregate. A single scalar hides exactly the compensating-error problem this protocol exists to prevent.

### 4.5 Algorithm

Stages 1 and 2 are fitted **jointly** rather than sequentially, because the in-cocoon emergence counter depends on degree-days accumulated from the end of in-nest development; the two stages are not separable in the way Stages 3 and 4 are separable from them.

Fitting uses differential evolution with a fixed seed, a population multiplier of 12 and **200 generations**. Member 0 of the initial population is the Formal Model vector, clipped into the box bounds; the remainder is a Latin hypercube. Because differential evolution retains the best member seen, seeding at the a priori parameterisation guarantees that the returned parameterisation is no worse than the published one. Seeding neither materially helps nor harms the fit — aggregate error differs by less than 0.01 from an unseeded search at every threshold floor — which indicates that the search explores the space adequately from either starting point.

The generation budget is not incidental. An earlier version of this work used 60 generations; at threshold floors near the collapse boundary that budget is insufficient, and the same floor returns different optima at 60 and at 200. All results reported here use 200, and floors were checked at both.

Two penalties supplement the error metric. Parameter sets that fail to clear the 31 December cull are penalised in proportion to the shortfall, using the worse of the two sites, so persistence is a first-class constraint rather than a post-hoc check. Parameter sets that drive the deterministic component of the emergence counter below one are also penalised, for the reason given in Section 4.9. **Every fit reported in this paper carries both penalties.**

### 4.6 The a priori parameterisation does not sustain the population

Evaluated against Pattern 1 with the implementation reconciled to the specification (Section 3.3), the Formal Model parameterisation collapses. No cohort completes in-nest development before the 31 December cull at either site.

| | Poznań egg | Poznań larva | Regensburg egg | Regensburg larva | Regensburg prepupa | Regensburg pupa |
|---|---|---|---|---|---|---|
| **Target** | 7.6 | 39.1 | 7.0 | 34.0 | 25.0 | 36.0 |
| Formal Model | 11.6 (+53%) | 51.4 (+31%) | 18.5 (+164%) | 39.7 (+17%) | 29.8 (+19%) | **297.7 (+727%)** |

Aggregate error is nRMSE 1.497 with 0% survival at both sites. Emergence onset, where it can be evaluated, is DOY 138 against an observed 104.

The pupal stage is the locus of failure: 297.7 days at Regensburg against a target of 36, and at Poznań the stage does not complete at all. This holds under each of the three weather series tested.

Because the cull encodes a biological fact (Section 2.2), this is not merely a poor fit. The published parameterisation predicts **local extinction within a single year** at three sites where *O. bicornis* is demonstrably present and persistent. It is falsified by the field, not merely disfavoured by the error metric.

### 4.7 The pupal base temperature is the single parameter responsible

Relaxing each Formal Model parameter individually to its calibrated value, holding all others at their published values, isolates the cause.

| Formal Model with… | nRMSE | Survival (Poznań / Regensburg) | Regensburg pupal stage |
|---|---|---|---|
| *(unmodified)* | 1.497 | 0% / 0% | 297.7 d |
| egg threshold 13.8 → 0.52 °C | 1.052 | 0% / 37% | 203.2 d |
| larva threshold 8.5 → 8.83 °C | 1.508 | 0% / 0% | 298.6 d |
| **pupa threshold 13.2 → 2.46 °C** | **0.541** | **94% / 100%** | **19.3 d** |
| pupa SET 272.3 → 555.9 DD | 1.759 | 0% / 0% | 355.5 d |
| prepupal scale 24.3 → 21.3 d | 1.487 | 0% / 0% | 295.5 d |
| emergence threshold 12 → 10.09 °C | 1.446 | 0% / 0% | 297.7 d |
| *all three in-nest thresholds together* | 0.418 | 100% / 100% | 17.4 d |
| *fully calibrated* | **0.039** | 100% / 100% | 36.3 d |

Three things follow.

**The pupal base temperature is the only single change that restores persistence**, taking survival from 0% to 94% and 100%. Neither the pupal degree-day requirement nor the prepupal scale does so.

**The larval threshold was already approximately correct.** Moving it from 8.5 to its calibrated 8.83 °C makes the fit very slightly *worse* (1.508 against 1.497). The a priori parameterisation is therefore not uniformly too high — a natural but incorrect reading of Section 4.6. The failure is confined to the egg and pupal thresholds, and the larval value stands as published.

**Changing a degree-day requirement without its threshold makes matters worse, not better.** Substituting the calibrated pupal SET alone raises the error from 1.497 to 1.759 and lengthens the pupal stage from 297.7 to 355.5 days. This is the clearest single demonstration in the paper that (*T*₀, SET) is one parameter in two coordinates: the calibrated SET is only meaningful alongside the calibrated threshold, and either taken alone is worse than neither.

### 4.8 The cost of constraining base temperatures [one-factor analysis]

All eleven parameters were then fitted jointly, seeded at the Formal Model. Because a linear degree-day model's base temperature is bounded below only by the data, we repeated the fit under a series of imposed lower bounds on every developmental threshold, to quantify what constraining those thresholds costs.

**What this analysis is.** It is a constrained-refit analysis, not a sensitivity analysis in the variance-based sense. It varies one parameter *class* along one axis, with all remaining parameters re-optimised at each point, and it therefore measures the cost of a constraint rather than the contribution of a parameter to output variance. No Morris or Sobol design re-optimises at each sample. Read as such, it is the strongest single statement this paper makes about model behaviour.

| Threshold floor | nRMSE | Egg *T*₀ / SET | Larva *T*₀ / SET | Prepupa d | Pupa *T*₀ / SET | Onset | Poznań | Regensburg |
|---:|---:|:---|:---|---:|:---|---:|---:|---:|
| **none (0 °C)** | **0.039** | 0.52 / 104.4 | 8.83 / 305.2 | 21.3 | 2.46 / 555.9 | 103 | **100%** | **100%** |
| 2 °C | 0.044 | 2.14 / 91.9 | 8.42 / 316.4 | 21.0 | 2.34 / 555.8 | 103 | 100% | 100% |
| 4 °C | 0.058 | 5.02 / 68.7 | 5.55 / 424.0 | 20.7 | 4.62 / 462.4 | 103 | 98% | 100% |
| 6 °C | 0.072 | 6.26 / 60.9 | 6.16 / 401.1 | 19.7 | 6.08 / 381.4 | 103 | 94% | 100% |
| 8 °C | 0.114 | 8.06 / 55.4 | 8.09 / 330.1 | 18.0 | 8.02 / 257.2 | 103 | 94% | 100% |
| 10 °C | 0.130 | 10.04 / 31.4 | 10.07 / 302.3 | 18.3 | 10.10 / 222.6 | 104 | 78% | 97% |
| **12 °C** | **0.555** | 12.01 / 30.1 | 17.32 / 521.6 | 20.6 | 14.68 / 242.1 | 103 | **0%** | **0%** |
| 13 °C | 0.578 | 13.03 / 30.0 | 16.27 / 623.2 | 55.9 | 13.79 / 319.7 | 103 | 0% | 0% |

Two things follow.

**There is a collapse boundary between 10 and 12 °C**, and the Formal Model's egg (13.8 °C) and pupal (13.2 °C) base temperatures lie beyond it. This explains Section 4.6 structurally rather than parametrically: no adjustment of degree-day requirements can rescue a parameterisation whose base temperatures exceed the boundary, which is exactly what Section 4.7 shows. Threshold upper bounds were widened to 18 °C for this analysis so that floors above 12 °C were feasible at all; the search could have gone higher had a solution existed, and did not.

**The cost of constraining base temperatures is continuous and substantial.** Aggregate error rises monotonically from 0.039 unconstrained to 0.130 at a 10 °C floor — more than threefold — before collapsing at 12 °C. Poznań persistence begins to erode from the 4 °C floor onward. The data prefer low base temperatures, and prefer them strongly.

**Robustness to the cull date.** The boundary is defined by a deadline encoded as a fixed calendar date, whereas the biological claim behind it concerns the onset of winter, which is not 1 January everywhere in every year. Repeating the analysis with the deadline shifted by ±30 days returns results identical to five decimal places, parameter values included; scanning the deadline from −60 to +90 days with parameters held fixed moves Poznań survival by at most 4 percentage points and Regensburg not at all. The a priori parameterisation requires the deadline to be extended by 150 days before a single cohort survives, and by 210 days for full survival, because its pupal stage runs 298–309 days against a target of 36. The calendar encoding is therefore not load-bearing for any result reported here.

### 4.9 Emergence onset alone admits degenerate solutions

Fitting emergence onset without further constraint proved degenerate. The optimiser drives the deterministic component of the emergence counter — `int(const + slope × accumulated DD)` — below one. Because the counter test is `--counter < 1`, the emergence-kernel draw (0–10 d) and the per-nest aspect delay (0–15 d) are then truncated away, and the entire cohort emerges on the first day at or above the emergence threshold. Onset matches its target while the distribution generating it is destroyed.

We therefore impose a structural constraint, not a tuned one: the deterministic counter component must be at least one, so that the two stochastic terms can act.

The degeneracy is not incidental to the threshold sweep; it is produced by it. Refitting without the constraint across the full range of floors:

| Threshold floor | 0 °C | 6 °C | 8 °C | 10 °C | 12 °C | 13 °C |
|---|---:|---:|---:|---:|---:|---:|
| Counter base, unconstrained | 36 | 21 | 11 | **−5** | **−14** | **−22** |
| Survival, Poznań / Regensburg | 100/100 | 94/100 | 94/100 | 77/94 | **37/73** | 0/0 |

The counter base falls monotonically as the floor rises, crossing zero between 8 and 10 °C and deepening thereafter. The mechanism is legible: as base temperatures are forced up, in-nest development slows and cohorts reach the cocoon stage later, so the optimiser can only recover a DOY-103 onset by collapsing the emergence distribution to a point.

The consequence for Section 4.8 is direct. **Without the constraint, a parameterisation appears to survive at a 12 °C floor** — 37% and 73% at the two sites, nRMSE 0.418. That parameterisation has a counter base of −14: its entire cohort emerges on one day. Persistence bought this way is an artefact of the fitting, not a property of the model. Imposing the constraint removes the apparent survivor and restores the collapse boundary to between 10 and 12 °C.

The episode is worth reporting because it is a compensating error arising *within* a single stage, against a single target — the failure mode the staged protocol was designed to catch between stages, occurring at a finer scale than the protocol anticipates.

### 4.10 The calibrated Stage 1–2 parameterisation

The recommended set is the unconstrained best fit. Full values are given in Appendix A, Table A2.

| | Poznań | | Regensburg | | | | Lusignan |
|---|---|---|---|---|---|---|---|
| | egg | larva | egg | larva | prepupa | pupa | onset |
| Target | 7.6 | 39.1 | 7.0 | 34.0 | 25.0 | 36.0 | 104 |
| Calibrated | 7.2 (−5.5%) | 39.3 (+0.4%) | 7.9 (+13.0%) | 34.6 (+1.9%) | 25.7 (+2.6%) | 36.3 (+0.8%) | 103 (−1.0%) |

Aggregate error is nRMSE 0.039, against 1.497 for the a priori parameterisation. Survival is 100% at both sites, and the modelled emergence distribution runs DOY 102–130 with a median of 115.

The residual pattern is informative. Errors are distributed in both directions across the two field sites — the egg stage runs short at Poznań and long at Regensburg — which is the intended behaviour given that the two datasets are mutually inconsistent (Section 4.13). No target is fitted at the expense of another. The largest single residual, +13.0% on the Regensburg egg stage, is also the target with the least thermal information behind it, since Regensburg's field temperatures are unrecorded and reconstructed.

### 4.11 Pattern 4 — female emergence mass

#### 4.11.1 The configured mass mapping is inconsistent with the field data

Regressing adult body mass on cocoon mass across the 1,305 paired records tests the model's composite mapping directly.

| | *n* | Fitted relation | Slope 95% CI | *R*² | Residual SD |
|:---|---:|:---|:---|---:|---:|
| Female | 601 | adult = 0.7471 × cocoon + 0.95 | [0.732, 0.763] | 0.936 | 4.54 mg |
| Male | 704 | adult = 0.7178 × cocoon + 0.61 | [0.698, 0.737] | 0.880 | 4.57 mg |
| *As configured* | — | *adult = 0.8118 × cocoon + 4.00* | — | — | — |

The configured slope lies well outside the confidence interval for either sex (females *t* = −8.10, *p* = 6 × 10⁻¹⁶; males *t* = −9.37, *p* < 10⁻¹⁶). The configured intercept is also rejected (females *p* = 0.002).

#### 4.11.2 The intercept is not distinguishable from zero, and should be zero

The configured intercept of 4.00 mg implies that a cocoon of zero mass yields a 4 mg adult, which is not interpretable. The data agree: the fitted intercept is 0.95 mg (SE 1.00) for females and 0.61 mg (SE 0.77) for males, neither distinguishable from zero (*p* = 0.34 and 0.43).

Constraining the regression through the origin costs nothing. Female *R*² is 0.9359 against 0.9360 for the two-parameter fit, with identical residual SD. The one-parameter model is preferred on both statistical and biological grounds:

  adult = 0.7546 (SE 0.0015) × cocoon,  *R*² = 0.936

#### 4.11.3 Recalibrated parameters, and a resolved inconsistency

Holding `OSMIAS_PROVISIONINGTOCOCOON` = 3.247 fixed gives slope 0.2324 (from 0.2500, −7.0%) and intercept 0.0 (from 4.00). The two-parameter alternative is slope 0.2301 with intercept 0.95; we prefer the origin-constrained form.

This recalibration closes the 12.4% provision-mass discrepancy reported in Section 3.4 exactly (residual difference +0.00%). The two conversions describe one physical relationship and were sourced independently — the multiplier from Seidelmann (2006), the slope and intercept pair from elsewhere — and were never checked against each other. This is the second instance in this paper of independently-sourced parameters proving mutually inconsistent, after the pupal threshold and degree-day requirement.

#### 4.11.4 Two findings not resolved by recalibration

**The sexes do not share a slope.** Female 0.7471 against male 0.7178, a difference of 0.0293 (SE 0.0128, *t* = 2.29, *p* = 0.022). The model applies one mass-from-provision relation to both sexes, so this is a structural gap rather than a parameter error. The effect is modest — about 4% of the slope — and we do not recommend adding a sex-specific parameter without independent confirmation, but it should be recorded.

**`OSMIA_MAXFEMALEMASS` is far above anything observed.** The configured maximum is 200 mg; the largest of 597 field females is 147.7 mg, 74% of that bound, and the 99th percentile is 128.0 mg. The bound is permissive rather than binding, which is harmless for the mass distribution but means the parameter carries no empirical content. `OSMIA_MINFEMALEMASS` = 25 mg is better placed: exactly one observation (24.4 mg) falls below it. See Section 3.5 for the interaction between that floor and the recalibrated slope, and for why it does not destabilise the implementation.

#### 4.11.5 An empirical provisioning distribution

Applying the fixed multiplier to the observed cocoon masses yields a field-grounded provision-mass distribution, with no simulation and no assumed forage input (Table A5). This supersedes the "standard good forage input" of 480 mg/day used in earlier testing, a figure that appears once in the project record with no derivation or citation. Against the distribution, 480 mg/day exceeds the median female cell requirement by 19% and the 95th percentile by 6%, confirming it was chosen to be non-limiting. That is appropriate for isolating foraging *behaviour* in Stage 3, but it is the least informative regime for Stage 4, since it removes precisely the forage limitation that generates the lower tail of the observed mass distribution.

#### 4.11.6 What regression cannot deliver

The parameters above fix the *mapping* from provision to adult mass. They do not reproduce the *distribution* of adult mass, whose spread (SD 17.36 mg, range 24.4–147.7) reflects variation in how much pollen each female actually collects. That is landscape- and forage-driven and requires the full model. The regression therefore calibrates Stage 4's deterministic core and leaves its stochastic component to be validated against the 597-record distribution once Stage 3 is complete.

### 4.12 Pattern 3 — eggs per nest, provisional

Ivanov (2006) sampled 352 completed nests of mixed order: mean 7.75 cells, SD 4.42, median 7, mode 4, range 1–21. The model's comparable quantity is the distribution of nest sizes **pooled across all nests of all females**, not the first-nest draw, because `Osmia_Female::st_ReproductiveBehaviour` decrements nest size by two for every subsequent nest. Comparing the first-nest draw instead reverses the sign of the discrepancy; we note it explicitly because it is an easy error to make.

**Female mass explains little of the observed variation.** The model's lifetime budget uses Seidelmann et al.'s (2010) regression of eggs per nest on female mass. Applied to the 597 observed female masses, that relation gives a mean of 6.28 eggs with an SD of only 0.64, against an observed SD of 4.42. Female mass therefore accounts for roughly 15% of the variation in cells per nest. The remaining 85% is supplied by the BETA draw in `PlanEggsPerNest()`, which is not a representation of any modelled process; it absorbs unmodelled variation, principally nest cavity geometry. The five studies tabulated in the project workbook report mean cells per nest between 5.6 and 12.2 depending on tube length, diameter and material, and Ivanov's own tubes ranged from 5 to 30 cm.

We take the same position here as in Section 4.14. The BETA parameters are **phenomenological, not mechanistic**: they describe the aggregate consequence of cavity heterogeneity that the model does not represent, and fitting them to a field distribution measured across heterogeneous cavities is legitimate provided that interpretation is stated. What is not legitimate is reading the fitted shape as a statement about female reproductive decisions.

**The joint fit also identifies `OSMIA_TOTALNESTSPOSSIBLE`.** Because the two egg-number routes must agree (Section 3.4), the BETA shape and `TOTALNESTSPOSSIBLE` are jointly rather than separately identified. Fitting both against Ivanov, with a penalty on relative disagreement between the ramp total and the Seidelmann budget, gives BETA shape 2.277 / 5.940 and `TOTALNESTSPOSSIBLE` 3.50, against configured values of 1.0 / 4.00 and 5 ("WIP find value"). Pooled nest size is 7.69 against an observed 7.75, SD 4.24 against 4.42, Wasserstein distance 0.329 against 2.728 as configured, and the lifetime routes agree to 5.1%. The consistency constraint identifies a parameter the Ivanov distribution alone could not.

**Why these values are provisional.** The fit matches *planned* nest size to *achieved* cell counts. Ivanov measured completed cells; the distribution fitted is the plan. Earlier testing reported a 75% planned-to-achieved failure rate. If that figure is real, planned nest size should substantially exceed achieved and this fit is invalid; if it is an artefact, the two are comparable. This single unresolved question determines whether the values can be adopted. Two further caveats: the definitional mismatch over single-egg nests remains, and two undocumented mechanisms are absorbed into the fitted values — the `+2` shift applied to 45% of nests, and the fact that `PlanEggsPerNest()` is called once per female rather than once per nest, so all between-nest variation derives from a single draw at emergence.

The route to resolution is a decomposition rather than a refit. The candidate loss pathways — provisioning shortfall, cell-construction time exhaustion, female mortality mid-nest, weather-limited flying days, and parasitism — are individually instrumentable in the full model, and quantifying them separately distinguishes a real biological loss from an accounting artefact. If the 75% figure is confirmed, Pattern 3 becomes a constraint on the *achieved* distribution and the planning parameters must be fitted through the loss model rather than against Ivanov directly; the values in Table A6 would then be superseded rather than merely refined. If it is an artefact, the present fit stands. This work requires the full model and is deferred with Stage 3.

### 4.13 Conflicting targets and how they were traded off

The pattern-oriented literature treats conflict between targets as informative rather than as a nuisance, and this calibration contains three distinct kinds.

**Two field datasets that disagree.** Poznań and Regensburg give different stage durations and cannot both be matched exactly. There is no basis for preferring one — different years, different countries, different observers, neither with usable temperature records — so the metric weights them equally as separate blocks and the fitted model distributes residuals in both directions (Section 4.10). The alternative, weighting by sample size or by apparent quality, would encode a preference we cannot justify.

**Laboratory against field.** The seven constant-temperature laboratory records constrain the shape of the thermal response; the two field records constrain totals under fluctuating temperature. These pull in opposite directions, and Table A3 shows the resulting compromise: the calibrated set is faster than the laboratory series at 20 °C for every stage while matching the field records closely. We resolved this in favour of the field records because the model is run under field forcing. That is a defensible choice rather than a neutral one, and it is the direct cause of the low fitted base temperatures.

**Fit against persistence.** These do not in fact conflict at the optimum — the best-fitting parameterisation also has 100% survival — but they conflict sharply near the collapse boundary, where the optimiser can buy persistence by destroying the emergence distribution (Section 4.9). The structural constraint resolves this by removing the degenerate region from the search space rather than by weighting the two objectives against each other.

### 4.14 Biological plausibility of the calibrated values

The interpretation of Section 4.8 turns on what the base temperature in a linear degree-day model represents. It is routinely read as the developmental zero — the temperature below which development physiologically ceases — and on that reading the fitted values of 0.52 °C for the egg and 2.46 °C for the pupa are not credible.

That reading is mistaken for this model. In a linear degree-day formulation the base temperature and the total degree-day requirement are strongly correlated and jointly identified: the same observed stage durations are reproduced by a continuum of (*T*₀, SET) pairs, and the fitted *T*₀ is the intercept of a linear approximation to a response that is not linear near its lower limit. It is a phenomenological fitting constant. Section 4.7's pupal-SET row is the direct demonstration: moving the requirement without the threshold makes the fit substantially worse.

Three further pieces of evidence support treating the pair as jointly identified rather than separately sourceable.

**The Formal Model sourced them separately, and the pair is internally inconsistent.** The pupal base temperature (13.2 °C) comes from a laboratory regression and the degree-day requirement (272.3) from a different source. Under field temperatures that pair is not attainable, which is the failure in Section 4.6. At a 10 °C floor the fitted requirement is 223 DD, not 272.

**The project's own base-temperature scan does not identify a single value.** Scanning candidate thresholds from 0 to 20 °C in 0.1 °C steps, the pooled degree-day variance minimises at 10.1 °C with a mean of 388.0 DD. Taken separately, however, the Radmacher & Strohm series minimises at 11.6 °C (341.0 DD, variance 25.3) and the Giejdasz & Wilkaniec series at 0 °C (704.7 DD, variance 1,843). The pooled optimum is a compromise between two datasets that do not agree on the base temperature, and its variance (2,829) is two orders of magnitude above the within-study optimum for Radmacher & Strohm.

**Laboratory-derived requirements overestimate field requirements at the same threshold.** The scan is fitted to constant temperatures of 17.5–30 °C; field temperatures are far cooler. The gap between the laboratory 388 DD and the field-fitted 223 DD at a comparable threshold is consistent with linear extrapolation failing at the cool end.

One caveat cuts the other way and should not be suppressed. The **larval** threshold is fitted at 8.83 °C, within 0.34 °C of the a priori 8.5 °C, and is the one threshold whose a priori value survives calibration essentially unchanged. Whatever is going wrong with the egg and pupal thresholds is therefore not a uniform property of the linear degree-day formulation; it is specific to those two stages, and a full explanation should account for the asymmetry rather than treating all three alike.

### 4.15 Stages not yet calibrated

**Stage 3 — foraging and provisioning.** No results. Earlier testing established one relevant point: under standard good-forage input, including female age in the female emergence size calculation has a negligible effect, with no visible change in the output histogram.

**Stage 5 — population-level consistency.** No results. Requires 20-year runs on all three weather series.

**Stage 6 — cross-country transfer.** No results. Three weather series are currently available (Lusignan FR, Poznań PL, Regensburg DE); Danish, Dutch and Finnish series must be sourced.

**Overwintering mortality.** `OSMIA_INCOCOONWINTERMORTCONST` and `SLOPE` are not fitted. The overwintering sub-model is separable in the same sense as the development code and could be fitted against Bosch & Kemp (2004) in the same framework at low cost. Those data are *O. cornuta*, and the cross-species assumption must be stated explicitly if they are used.

**Emergence spread.** Not fitted, and Section 4.9 shows that onset alone admits degeneracy. Every available spread target is imperfect: the Lusignan spread is confounded by adult lifespan and detectability; the Giejdasz & Wasilewski kernel is a laboratory cohort and the model already uses its widest treatment; and the Kehrberger & Holzschuh window has no weather series. The most defensible route is to fit spread against the Lusignan curve *with* an explicit adult survival model layered on top, so the confound is modelled rather than ignored — which requires the adult stage, and therefore full ALMaSS.

---

## 5. Validation

### 5.1 Performance against held-out data

Kehrberger & Holzschuh (2019) recorded *O. bicornis* emergence in calcareous grassland near Würzburg, Germany, in 2015, giving an emergence start window of DOY 100–106. These data were **not used in fitting**: no weather series exists for that site, so the record cannot enter the objective function, and it was reserved as an independent check.

The calibrated parameterisation gives an emergence onset of DOY 103, within that window. This is a genuine out-of-sample result in the sense that matters — a different country, a different year, a different observer, and a quantity the optimiser never saw — though it should not be over-claimed. The window is 7 days wide, the Lusignan onset target used in fitting is DOY 104, and the two sites are climatically not very distant. It is a consistency check that the model passed and could have failed, not a demanding test.

### 5.2 Emergent patterns not explicitly fitted

Three model behaviours were not targeted and are therefore available as weak validation.

**Population persistence.** Survival over the 31 December deadline is imposed as a penalty during fitting but is not a fitted quantity: nothing in the objective rewards 100% over 95%. The calibrated set achieves 100% at both sites and remains above 94% under a deadline shifted by a full month in either direction (Section 4.8).

**Residual sign structure.** The two field datasets are mutually inconsistent, so a well-behaved fit should distribute errors in both directions rather than tracking one site and abandoning the other. It does (Section 4.10). This was not enforced by the metric — equal block weighting permits a solution that fits one site and takes the loss on the other — so it is a property the fit exhibited rather than one it was required to exhibit.

**A parameter recovered by consistency rather than fitted.** `OSMIA_TOTALNESTSPOSSIBLE` is identified by requiring the model's two egg-number routes to agree, not by matching any distribution (Section 4.12). The Ivanov data alone cannot identify it. That a purely internal consistency requirement yields a value in a plausible range is weak evidence that the surrounding structure is coherent.

### 5.3 Performance under novel conditions

**This cannot yet be tested.** Only three weather series are available, all from temperate continental western and central Europe, and all three were used in fitting or diagnosis. The transferability that Stage 6 is intended to establish — Denmark, the Netherlands, Finland — requires weather series that have not been sourced. Section 6.1 gives a specific reason to expect the linear degree-day formulation to transfer poorly to colder sites, which makes this the most consequential outstanding test in the protocol.

### 5.4 What remains unvalidated

The Stage 1 and Stage 2 results were produced by a standalone reimplementation of the in-nest development and emergence code. The extraction is exact in principle, since the extracted code reads only temperature and configuration, and it is validated against C++ semantics by a test suite (Section 3.2). It has **not** been cross-checked against a full ALMaSS run. This matters more than it would otherwise, because the operational code has been amended (Section 3.3) and the parameterisation reported here has therefore never been run in the full model.

Until that check is performed, the Section 4 results should be treated as provisional. It is the one property the approach cannot self-validate, and it is the first item of outstanding work.

Two further limitations are worth stating in the same place. Emergence spread is not fitted, and the structural constraint of Section 4.9 prevents the specific degeneracy observed but is not a substitute for a spread target; we cannot rule out other degenerate regions it does not exclude. And neither Pattern 1 field dataset records the temperature under which it was collected in a form usable for degree-day fitting, so site temperature is reconstructed from the weather series for the stated study year, introducing an error we have not quantified.

---

## 6. Structural evaluation

### 6.1 Linear degree-days and the case for non-linear development

The calibration prefers base temperatures near zero for the egg and pupa (Section 4.8), and Section 4.14 argues this reflects the linear degree-day model absorbing curvature at the cool end into its intercept rather than a claim about physiology.

The structural implication is that **linear degree-day accumulation is a poor description of development near the threshold**, and that a non-linear rate function would be the principled remedy. The model already concedes this for the prepupa, which uses a quadratic rate function precisely because its temperature response is non-monotonic. The asymmetry — one stage with a fitted non-linear response, three with linear degree-days — is not biologically motivated. It reflects the availability of a published curve for the prepupa rather than any claim that the other three stages respond linearly.

Extending non-linear treatment to the egg, larva and pupa would let base temperature recover its physiological interpretation and would likely improve transferability to the colder countries in Stage 6, where the linear approximation is worst. This is a recommendation for the next Formal Model revision, not a change we make here.

### 6.2 The day-0 cull: a modelling decision checked against the biology

The 31 December cull looks at first inspection like an implementation convenience — a housekeeping step in `DoBefore()` that clears the stage lists at year end. Because it is used as a first-class constraint throughout Section 4, it was interrogated rather than assumed.

Two lines of evidence support it as a biological statement. The implementation applies it to exactly the three stages that use temperature-driven somatic development and exempts cocooned adults and adult females, which matches *O. bicornis* overwintering as a cocooned adult; a convenience-motivated cull would have no reason to respect that boundary. And the model authors confirm the intent: an individual that has not reached the cocooned-adult stage by the onset of winter is dead.

The cull is therefore correct, and the persistence constraint it generates is admissible evidence rather than a numerical device. Two consequences follow: Section 4.6 is a biological falsification rather than a goodness-of-fit result, and persistence takes its place in the pattern hierarchy as a genuine population-level pattern (Section 4.1).

One approximation remains, and it is worth naming precisely because the biology has been confirmed. What the authors endorsed is the biology; what the code asserts is narrower — that the onset of winter is 1 January, everywhere, in every year. It is not, and the population manager already performs temperature-driven winter-onset detection for the pre-wintering transition, so a mechanism for doing better exists in the same class. We tested the approximation rather than defending it, and it does not matter here: shifting the deadline by ±30 days with full re-optimisation returns identical results, and the a priori parameterisation needs 150 days of extension before a single cohort survives (Section 4.8). The calendar encoding should nonetheless be replaced with the temperature-driven test in the next revision, since a robustness result on three temperate series is not a guarantee for the colder sites Stage 6 will add.

This subsection is also a small worked example of the point the editorial makes about the testing-to-implementation loop: a decision that looked like a convenience was examined, found to be biologically motivated, and its one remaining approximation quantified rather than assumed away.

### 6.3 Independently-sourced parameters cannot be assumed mutually consistent

The Formal Model is a careful, peer-reviewed synthesis of the available literature, and its parameterisation does not work. The cause is narrow and identifiable: a single base temperature, sourced from a laboratory regression, lying beyond the boundary at which the population can persist.

The general lesson is not that the synthesis was careless. It is that **parameters describing a single relationship cannot be sourced independently of one another.** Base temperature and total degree-day requirement are jointly identified by the data; taking one from a laboratory regression and the other from a different study produces a pair that may be internally inconsistent, and that inconsistency is invisible until the model is run under field forcing.

This paper contains three independent instances, which is what persuades us the failure mode is general rather than incidental:

1. **The pupal pair** (Sections 4.6, 4.14), where the inconsistency manifests as population collapse.
2. **The mass chain** (Sections 3.4, 4.11), where the cocoon-to-provision multiplier and the provision-to-adult-mass relation give provision masses differing by 12.4% on the same 601 individuals.
3. **The nest-density parameters** (Section 6.6), where the cited source cannot supply the quantity the model reads from it.

The second case is the most instructive because it was *silent*: it produced no collapse, no obviously wrong output, and would not have been detected by any amount of pattern-matching on model behaviour. It surfaced only when both conversions were applied to the same measured individuals and compared. That suggests a concrete and cheap check for Formal Models generally — wherever two or more parameters jointly determine one observable relationship, evaluate them against a common dataset before publication.

This has a direct implication for the Formal Model format itself: a Formal Model that specifies parameters individually, with individual citations, has no mechanism for checking joint consistency. Some form of coherence check under realistic forcing, before publication, would have caught all three.

### 6.4 Processes needing better representation

**Cavity geometry is absorbed into a distribution.** Roughly 85% of the variation in cells per nest is supplied by a BETA draw standing in for nest cavity heterogeneity that the model does not represent (Section 4.12). Since cavity availability is exactly what a nest-provision or habitat-management scenario would alter, a model intended for such scenarios should represent cavity dimensions rather than absorbing their consequences into a fitted shape.

**The mass-from-provision relation is not sex-specific**, though the data show the sexes differ in slope by about 4% (Section 4.11.4).

**Planned-to-achieved loss is not decomposed.** The reported 75% failure rate is either a major unmodelled loss process or an artefact, and Pattern 3 cannot be finalised either way until the pathways — provisioning shortfall, cell-construction time exhaustion, female mortality mid-nest, weather-limited flying days, parasitism — are quantified separately.

### 6.5 An alternative structural treatment: nest limitation in SolBeePop

SolBeePop (Schmolke et al. 2023) and its regulatory extension SolBeePop_ecotox (Schmolke et al. 2024) address a closely related domain — solitary-bee population dynamics for pesticide risk assessment, parameterised for *O. bicornis* among others — and treat nest limitation on a different structural assumption. The comparison is instructive precisely because the two models were built for the same purpose and diverge.

| | SolBeePop_ecotox | ALMaSS *Osmia* |
|:---|:---|:---|
| Spatial representation | Non-spatial; the landscape enters as a daily floral-resource time series | Fully spatial; every polygon carries its own capacity |
| Currency of the limit | Occupied brood cells, population total | Nests, per polygon |
| Functional form | **Graded** — cell production rate declines linearly from 1 at a threshold to 0 at a maximum | **Binary** — a hard cap plus a constant Bernoulli gate |
| What it acts on | Cell production rate of every nesting female | Whether a female may found a nest in this polygon |
| Habitat differentiation | None | Per landscape element type (124 types) |
| Temporal variation | Thresholds may vary per simulated year | Drawn once at initialisation, fixed thereafter |
| Provenance of the values | Round assumed numbers (250 and 2,500 cells), identical across all four species modelled | Koh-derived, rescaled by an undocumented constant (Section 6.6) |
| How the values are defended | Latin hypercube sampling with partial rank correlation analysis, published | Presented as literature-derived |

Three observations follow, and only the first is about parameter values.

**The graded form is the transferable idea, and it addresses a defect identified independently.** ALMaSS gates nest founding on `(m_CurrentOsmiaNests < m_MaxOsmiaNests) && (g_rand_uni_fnc() < m_OsmiaNestProb)` — a wall plus a constant thinning probability. Section 6.6 shows that the wall is effectively non-binding in woody habitat and exactly zero in grassland, so nest-site availability is currently closer to a binary habitat classification than to a graded resource. Making the nesting probability a declining function of occupancy, as SolBeePop does for cell production, would produce graded crowding instead of that dichotomy, and would make population outcomes markedly less sensitive to the absolute magnitude of a cap we have argued is not empirically grounded. It is a small change to one function.

**We do not recommend importing their values.** The currency differs — brood cells against nests — the spatial logic differs, and the numbers are assumed rather than measured. The comparison is nonetheless calibrating in one respect: at roughly 20 cells per nest, SolBeePop's maximum of 2,500 occupied cells corresponds to about 125 nests for an entire population, against ALMaSS's nominal 513,400 nests per hectare of deciduous forest before the debug scaler and roughly 740 after it. Two models built for the same regulatory purpose differ by orders of magnitude in what they assume nest availability to be, and neither has measured it.

**Their treatment of the resulting uncertainty is the better model, and we should adopt it.** SolBeePop uses round assumed numbers and then demonstrates through sensitivity analysis how much they matter. That is a stronger scientific position than two-decimal-place values presented as literature-derived, and it points to the most efficient next step for this model: run a Latin hypercube over the nest-density scaler across two or three orders of magnitude before spending further effort sourcing per-habitat values. If population output proves insensitive across that range, the question raised in Section 6.6 is less urgent than it appears — and that is itself a reportable finding. If it proves sensitive, the effort is justified and targeted.

**The repository also offers validation data.** SolBeePop bundles, under GPL-3.0, six *O. bicornis* field trials in German oilseed-rape landscapes (Ruddle et al. 2018) with site weather and aggregated land cover at three levels of detail, together with semi-field datasets recording nest occupancy, brood-cell production, cocoon output and emergence. These are the same emergent quantities the outstanding Stage 3 and Stage 5 work must be evaluated against, and they carry landscape context that the present targets lack. Their overlap with the data already in `calibration_data/` has not been checked; if independent, they constitute a ready-made validation set rather than another calibration set, and would address the gap identified in Section 5.3 more cheaply than sourcing new weather series. This is the single most valuable outstanding opportunity identified in this review.

### 6.6 The nest-density parameters are calibrated, not literature-derived

Nest-site capacity per habitat type is read from `OsmiaNestsByHabitat.txt` and traced by the Formal Model's parameter table to Koh et al. (2016). The provenance chain does not support that attribution.

**The source is a relative index with no absolute scale.** Koh et al. elicited nesting suitability from 14 experts across 45 US land-cover classes and four nesting guilds, each expert choosing one of five values on a dimensionless 0–1 scale, summarised across experts *and* across guilds with guilds weighted by species proportions — ground-nesters 70%, cavity-nesters 20%. The index enters the underlying Lonsdorf model only as a multiplicative weight, so multiplying every value by any positive constant leaves predicted relative abundance unchanged. The documentation states the limitation explicitly: the model "only estimates *relative* patterns of pollinator abundance … because absolute estimates of nest density … are rarely available."

Table 5's conversion step — "suitability of 1 equals … ~100 cells available per m²" — is therefore not a conversion but a new, unsourced parameter, and it is the only thing setting the absolute magnitude of nest availability in the entire model. Three further assumptions are added along the chain and none is supplied by the source: that a US land-cover translation applies to Danish ALMaSS habitat types, that a ~70% ground-nester-weighted index describes a cavity-nesting species, and that an expert-elicited quartile can be read as a density bound.

**The magnitudes are not attainable.** Taken literally, Table 5 gives a *floor* of 51.34 nests m⁻² for deciduous forest — 513,400 nest sites per hectare — and 45.22 m⁻² for hedges and scrub. No measured *O. bicornis* density is within three orders of magnitude. The code compensates with a hard-coded literal at `Osmia_Population_Manager.cpp:1013`:

```cpp
double n = (minOsmiaNests[found] + g_rand_uni_fnc() * (maxOsmiaNests[found] - minOsmiaNests[found])) * 0.001;
m_PolyList[e].SetMaxOsmiaNests(n); // ***CJT*** Added 0.001 scaler here to reduce densities for debug
```

Realised capacity is one thousandth of the tabulated values: deciduous forest 513–973 nests ha⁻¹, hedges ~695 ha⁻¹. Those are defensible numbers, which is precisely the difficulty — a scaler labelled *debug* is carrying the parameterisation, and the file values consequently do not correspond to Table 5's stated units.

**The operational values also diverge from Table 5.** Of 73 comparable rows, 54 match exactly; 19 diverge. Eighteen are zeroed, eliminating four Koh habitat classes entirely — Grass, Pasture/Hay, Grassland Herbaceous and Flowers, covering road verges, unsprayed field margins, permanent set-aside, beetle banks, field boundaries, permanent pasture and natural grassland. Because `InitOsmiaBeeNesting` sets `m_PossibleNestType[tole] = (min > 0)`, a zeroed row does not lower capacity but removes the habitat type from the nesting set altogether. These are exactly the agri-environment features a landscape-scale risk assessment exists to discriminate between. The one element moved upward, HedgeBank, is defensible on its merits but likewise undocumented. None of the 19 divergences appears in the deviations section of the implementation documentation.

**Table 5 also carries three internal errors**, which should be noted in any erratum: DrainageDitch has min 0.12 > max 0.00 (the max should read 31.58); Wasteland shows all three suitability columns as absent yet reports densities; and Vildtager has a 75th percentile below its mean.

**A second-order consequence.** With the 0.001 scaler, `SetMaxOsmiaNests` truncates to integer, so a polygon needs density × area ≥ 1 for any capacity at all. A garden or parkland polygon drawn near its minimum requires ≥ 1,493 m² to support a single nest and is silently non-nesting below that. Small urban and linear polygons are therefore excluded on top of the zeroing above. Forest is unaffected.

**The conclusion is the same as in Sections 4.14 and 6.3.** The nest-density parameters are phenomenological quantities fitted to produce sensible behaviour, not literature-derived values. Only the *ranking* of habitats comes from Koh et al.; the magnitude is assumed, and then rescaled by an undocumented constant. That is a legitimate modelling position provided it is stated — which is the whole of our recommendation. Specifically: document the zeroing decision or reverse it; promote the `0.001` literal to a named configuration variable; describe the file values as nominal rather than measured; and report realised nests ha⁻¹ by habitat from a test run, since the Koh-derived caps are non-binding in woody habitat and exactly zero in grassland, making nest-site availability effectively binary rather than graded. That last point undercuts the nest-limitation premise the model is built on, and it deserves resolution before any habitat-management scenario is run.

---

## 7. Uncertainty, equifinality and confidence

The editorial requires transparency about uncertainty, equifinality, and which domains are well supported against which remain speculative. Dropping the formal uncertainty analysis (Section 1.4) makes this section more important, not less.

### 7.1 Joint identifiability of (*T*₀, SET)

The clearest equifinality in this work is structural rather than incidental. In a linear degree-day model the same observed stage durations are reproduced by a continuum of (*T*₀, SET) pairs: raising the threshold and lowering the requirement together leaves predicted durations under a given weather series nearly unchanged. Section 4.8's table is a traverse along that continuum, and the fit degrades gently across most of its range — from 0.039 to 0.130 over ten degrees — before failing abruptly.

The practical consequence is that no single fitted threshold should be reported as *the* base temperature of a stage. What the data identify is a curve, and the paper's substantive claim concerns where that curve ends (Section 4.8), not where along it the optimum happens to sit.

### 7.2 The emergence-counter degeneracy

A second and more troubling equifinality is documented in Section 4.9: parameterisations that match emergence onset exactly while collapsing the emergence distribution to a single day. This is not a benign trade-off along a well-behaved ridge — it is a region where a fitted quantity is matched by destroying the mechanism that generates it.

We exclude it by a structural constraint. Two honest caveats: the constraint was devised after the degeneracy was observed rather than derived in advance, and we cannot rule out other degenerate regions it does not exclude. A spread target would resolve the matter properly and does not currently exist (Section 4.15).

### 7.3 A rounding choice worth 33% of lifetime output

`OSMIA_TOTALNESTSPOSSIBLE` is fitted at **3.50** and the parameter is integer-typed, so the configuration file must carry 3 or 4. Total reproductive output scales linearly with this value, so the rounding choice is worth a 33% swing in lifetime egg production. Neither 3 nor 4 is "the fitted value".

This should be resolved deliberately rather than by rounding convention — either by changing the parameter to a float, or by refitting with the value constrained to integers so the remaining parameters can compensate. Until then, the parameter file carries 4 and this paragraph is the disclosure.

### 7.4 Where uncertainty is quantified, and where it is not

| Result | Uncertainty quantification |
|:---|:---|
| Pattern 4 mass mapping (Section 4.11) | **Genuine.** Slope 0.7546 with SE 0.0015, 95% CIs on both sexes, residual SD 4.54 mg, formal *t*-tests rejecting the configured values |
| Stages 1–2 development and emergence (Section 4.10) | **None.** Point estimates only. Table A2's "range explored" column is a search bound, not an interval |
| Threshold sweep (Section 4.8) | A **sensitivity envelope**, not a confidence region — it shows how the optimum moves under an imposed constraint, not how uncertain the optimum is |
| Pattern 3 eggs per nest (Section 4.12) | **None**, and provisional in addition |

The asymmetry is a consequence of method rather than of effort. Pattern 4 is estimated by regression, which supplies standard errors as a by-product; Stages 1–2 are estimated by global optimisation against a simulator, which does not. Bootstrapping the Stage 1–2 fit over resampled targets is feasible — the fit takes seconds — but with only two field records per site the resampling distribution would be dominated by which of two mutually inconsistent datasets happened to be drawn, and would overstate its own precision. We prefer to report point estimates with the sweep as an envelope, and to say so.

### 7.5 What is well supported and what is speculative

**Well supported.** That the a priori parameterisation cannot sustain the population under the three weather series tested; that the pupal base temperature is the parameter responsible; that base temperature and degree-day requirement are jointly identified; that the configured mass mapping is inconsistent with the paired field data. Each rests on a direct comparison with data or on an internal consistency check, and each is robust to the analytical choices we varied.

**Moderately supported.** The specific calibrated values of Section 4.10, which fit well and pass an out-of-sample check but rest on two mutually inconsistent field datasets with reconstructed temperatures, and which have not been run in the full model.

**Speculative.** Everything downstream of Stage 3. The Pattern 3 values are provisional in a strong sense (Section 4.12). Population-level behaviour is untested. Cross-country transferability is untested and Section 6.1 gives a specific reason to doubt it. The nest-density parameterisation is, as Section 6.6 shows, not empirically grounded in its absolute magnitude at all.

No prediction from this model at the population scale should currently be treated as quantitative.

---

## 8. Discussion

### 8.1 What this means for the Formal Model format

The Formal Model concept exists to make modelling intent and its data foundation reviewable before implementation, and it does that well: the *Osmia* Formal Model is why this paper could evaluate an a priori parameterisation at all, rather than comparing one set of tuned values against another. The failure documented here is not a failure of the format's purpose but a gap in its machinery.

A Formal Model specifies parameters individually, each with its own citation, and there is no step at which the resulting set is checked for internal coherence. Three times in this paper, parameters that were correctly sourced individually proved mutually inconsistent (Section 6.3). Once the inconsistency produced population collapse and was unmissable; once it produced a silent 12.4% discrepancy that no amount of behavioural testing would have surfaced; once it produced a parameter attributed to a source that cannot supply the quantity read from it. The common structure is that two or more parameters jointly determine one observable relationship, and were sourced as though they were independent.

Two additions to the format would have caught all three, and both are cheap.

**Record which parameters are jointly identified.** A Formal Model parameter table already carries a source column. A column stating what each parameter is jointly identified *with* would make the (*T*₀, SET) pairing explicit, flag the cocoon-to-provision and provision-to-adult multipliers as one relationship in two parts, and force the question of whether the two sources are compatible. Where a parameter is a fitting constant rather than a measurable property — as we argue the base temperatures are (Section 4.14) — that column is where to say so.

**Check coherence under realistic forcing before publication.** The pupal failure required only that the specified parameterisation be run against a real weather series and the resulting stage durations compared with the field records the Formal Model itself tabulates. That is hours of work, not a calibration exercise, and it does not need the full model — the separable-sub-model extraction of Section 3.1 was itself assembled in days. A Formal Model that has cleared such a check is a substantially stronger object than one that has not.

We would add a third suggestion from the other direction. The nest-density case (Section 6.6) is not a joint-consistency failure but an attribution failure: a source that supplies a *relative* index was cited for an *absolute* quantity, and the conversion factor that bridged them was introduced without a source. A parameter table that distinguished "measured", "estimated from literature", "derived under stated assumptions" and "assumed" would have made that visible at review. The current convention collapses all four into "estimated from literature".

### 8.2 What this means for the model

Three changes are recommended, in order of how much they would alter the model's behaviour.

**Non-linear development for the egg, larva and pupa** (Section 6.1). The model already concedes the principle for the prepupa. The asymmetry reflects the availability of a published curve rather than a claim that the other stages respond linearly, and it is the direct cause of base temperatures that cannot be read physiologically. Extending the treatment would let base temperature recover its interpretation and should improve transfer to the colder countries Stage 6 will add, where the linear approximation is worst.

**A documented nest-density parameterisation** (Section 6.6), with the graded density-dependence of Section 6.5 in place of the current binary cap. The two go together: a graded response is less sensitive to the absolute magnitude of a cap we cannot source, so adopting it reduces the cost of the parameterisation problem rather than merely relabelling it.

**Resolution of the planned-to-achieved question** (Sections 4.12, 6.4). This is not a modelling change but a diagnostic that must be run, and it gates whether Pattern 3 can be fitted at all.

Two smaller items should not be lost: the `std::exit()` on the female mass sanity check (Section 3.5) is currently unreachable because the provisioning target and the founding cohort are both derived from the configured mass mapping, but it would become a hard termination again if those ever diverged, and a survival outcome would be the safer response; and the day-0 cull should be keyed to the temperature-driven winter onset the population manager already detects rather than to 1 January (Section 6.2).

### 8.3 What this means for risk assessment

The model is not yet usable for quantitative population-level risk prediction, and it is worth being precise about why, because the reasons are specific and addressable rather than fundamental.

Stages 1, 2 and the deterministic core of Stage 4 are calibrated and, within their scope, well supported. What is missing lies exactly where a risk assessment would put its weight. Foraging and provisioning are unfitted, so the pathway from landscape change to reproductive output — the pathway an exposure scenario acts through — is uncalibrated. Population-level consistency has not been tested, so the model's multi-year behaviour is unknown. The nest-density parameterisation, which determines how habitat quality translates into carrying capacity, is not empirically grounded in its magnitude and is currently near-binary across habitat types, which undercuts the nest-limitation premise a habitat-management scenario would rely on. And transferability beyond three temperate sites is untested, with a specific structural reason to expect it to be poor at colder ones.

Two things the model can already support are worth stating positively, since a purely negative assessment would be misleading. Its phenology is calibrated against field records at two sites and passes an out-of-sample emergence check at a third, so questions about *timing* — the overlap between adult activity and an application window, for instance — rest on the best-supported part of this work. And the model is now a usable instrument for structural inquiry: the analysis in Section 4.8 is exactly the kind of question a risk assessment needs answered about its own assumptions, and it took seconds to run once the sub-model was extracted.

The honest summary is that the model is at the stage where its mechanisms can be interrogated but its population predictions cannot yet be believed, and the work required to change that is enumerated in Sections 4.15 and 5.3 rather than open-ended.

### 8.4 A note on publication route

This paper appears as an appendix to the implementation documentation rather than as a standalone article. That suits a first calibration, where the material is inseparable from the implementation it evaluates — a third of this paper concerns implementation defects found by the testing process, and separating it from the documentation of that implementation would serve no one.

The editorial's rationale for separate publication nonetheless applies in the other direction and should be recorded. Recalibration for a new region ought not to require duplicating implementation documentation, and Stage 6 anticipates exactly that. When the model is recalibrated for Denmark, the Netherlands or Finland, that work will need either a standalone Testing and Calibration paper or a versioned update to this appendix. The present arrangement is appropriate for the first pass and should not be assumed to be permanent.

---

## 9. Conclusions

We verified the ALMaSS *Osmia bicornis* implementation against its Formal Model, calibrated its temperature-driven development and emergence against a hierarchy of field patterns, and evaluated the resulting model structurally. Five conclusions follow.

**The published a priori parameterisation does not sustain the population.** Under three European weather series it predicts that no cohort completes in-nest development before winter, at sites where the species is present and persistent. This is a biological falsification rather than a poor fit, and it holds after the implementation was reconciled with the specification. Aggregate error against the field stage durations is nRMSE 1.497, against 0.039 for the calibrated set.

**A single parameter is responsible, and the diagnosis is precise.** The pupal base temperature is the only individual change that restores persistence, taking survival from 0% to 94% and 100%. The larval threshold, by contrast, was already approximately correct and survives calibration essentially unchanged. Changing the pupal degree-day requirement without its threshold makes the fit substantially *worse*.

**Base temperature and degree-day requirement cannot be sourced independently.** They are two coordinates on one curve, and the calibration traverses that curve: fit degrades gently across ten degrees of imposed constraint before failing abruptly between 10 and 12 °C — beyond which the Formal Model's egg and pupal values lie. The same failure recurs in the mass chain, where two independently sourced conversions disagree by 12.4% on the same individuals, and in the nest-density parameters, where a relative index has been read as an absolute density. Three instances persuade us the failure mode is general to parameter-by-parameter literature synthesis rather than particular to this model.

**Fitting can buy an apparent result by destroying the mechanism that produces it.** Constraining base temperatures upward drives the optimiser toward collapsing the emergence distribution to a single day, and the degeneracy deepens monotonically with the constraint. Without a structural guard, a parameterisation appears to persist at a 12 °C floor; with it, the collapse boundary stands. This is a compensating error arising within one stage against one target — finer-grained than the staged protocol was designed to catch.

**The calibration is incomplete, and the incompleteness is where it matters most.** Stages 1, 2 and the deterministic core of Stage 4 are fitted. Foraging, population-level consistency and cross-country transfer are not, and the Stage 1–2 results await a full-model cross-check. The model can currently be used to interrogate its own structure; it cannot yet support quantitative population-level prediction.

The general recommendation is procedural. A Formal Model should record which of its parameters are jointly identified, should distinguish measured from assumed values in its parameter table, and should be run once under realistic forcing before publication. Each of the three inconsistencies reported here would have been caught by that, at a cost measured in hours.

---

## Appendix A — Parameter tables

### Table A1. Pattern 1 parameters: definitions and provenance

Parameters governing temperature-driven development of the four in-nest life stages. In-cocoon parameters govern Pattern 2 and are tabulated separately; note however that all five stages were fitted jointly.

| Parameter | Symbol | Description | Units | Source (Formal Model) | Notes |
|:---|:---:|:---|:---|:---|:---|
| `OSMIA_EGGDEVELTHRESHOLD` | *T*₀,egg | Lower developmental threshold below which egg development ceases | °C | Estimated from literature: Giejdasz & Wilkaniec (2002), Radmacher & Strohm (2011), Giejdasz & Fliszkiewicz (2016). Ziółkowska et al. (2023), Table 3 | Thresholds chosen to minimise covariance across the laboratory series. See Section 4.14 on interpretation |
| `OSMIA_EGGDEVELDD` | *SET*egg | Sum of effective temperatures above *T*₀ required for the egg to hatch | degree-days | As above | Jointly identified with *T*₀,egg |
| `OSMIA_LARVADEVELTHRESHOLD` | *T*₀,larva | Lower developmental threshold for larval development | °C | As above | The one a priori threshold that survives calibration essentially unchanged (Section 4.7) |
| `OSMIA_LARVADEVELDD` | *SET*larva | Sum of effective temperatures required for the larva to become a prepupa | degree-days | As above | |
| `OSMIA_PREPUPADEVELDAYS` | *D*prepupa | Prepupal stage duration at the thermal optimum; scales the rate function | days | Giejdasz et al. (2016), via Ziółkowska et al. (2023), p.12 | Not a degree-day quantity. The stage takes exactly this many days at a constant 22 °C |
| `OSMIA_PREPUPALRATE_A/B/C/TOPT` | *r*(*T*) | Daily prepupal development rate as a function of temperature, normalised to 1.0 at the optimum | dimensionless (0–1] | Derived: reciprocal of the Formal Model quadratic, a mean of the two functions in Ziółkowska et al. (2023), Fig. 3C | See Table A1b. Not fitted |
| — (prepupal variation) | — | Individual variation around *D*prepupa, drawn uniformly per individual | proportion | Assumed; Ziółkowska et al. (2023) | ±10%. Not fitted |
| `OSMIA_PUPADEVELTHRESHOLD` | *T*₀,pupa | Lower developmental threshold for pupal development | °C | Estimated from literature as above. Ziółkowska et al. (2023), Table 3 | The parameter responsible for the failure in Section 4.6 |
| `OSMIA_PUPADEVELDD` | *SET*pupa | Sum of effective temperatures required for the pupa to become a cocooned adult | degree-days | As above | Jointly identified with *T*₀,pupa |

### Table A1b. The prepupal rate function, stated explicitly

Prepupal duration is

  *d*(*T*) = *D*prepupa · *q*(*T*) / *q*(*T*opt),  *q*(*T*) = *aT*² + *bT* + *c*

equivalently a daily development rate *r*(*T*) = *q*(*T*opt)/*q*(*T*) accumulated until it exceeds *D*prepupa.

| Coefficient | Symbol | Value | Units | Source | Notes |
|:---|:---:|---:|:---|:---|:---|
| Quadratic, squared term | *a* | 0.0149431912 | °C⁻² | Derived from Ziółkowska et al. (2023), Fig. 3C | Only the ratios *a*:*b*:*c* are identified; the function is normalised at *T*opt |
| Quadratic, linear term | *b* | −0.6679153638 | °C⁻¹ | As above | |
| Quadratic, constant term | *c* | 8.4616334666 | dimensionless | As above | |
| Normalisation temperature | *T*opt | 22.0 | °C | Ziółkowska et al. (2023), p.12 | The stated optimum. The analytic vertex is at 22.35 °C, where duration is 24.256 d rather than 24.300 d |

Recovered by fitting a quadratic to the reciprocal of the published 42-entry array: maximum absolute residual 1.9 × 10⁻⁸, and reconstructing the array as *q*(22)/*q*(*T*) reproduces every published value to within 9 × 10⁻¹⁰. Scaling by *D*prepupa = 24.3 d gives duration directly as 0.363120·*T*² − 16.230343·*T* + 205.617693 days, which returns exactly 24.3000 d at 22 °C.

Because the discriminant *b*² − 4*ac* = −0.0597 with *a* > 0, *q*(*T*) is strictly positive at every real temperature (minimum 0.998 at the vertex), so the rate is always finite and no clamping is required. The implementation verifies both conditions at startup, since the coefficients are configurable.

The upstream fit behind Fig. 3C is **not** recoverable from the project files. Ziółkowska et al. (2023) describe the function as a mean of two quadratics fitted separately to Radmacher & Strohm (2011) and Giejdasz & Fliszkiewicz (2016), but neither component fit nor the averaging step is recorded. Given that the two studies disagree elsewhere (Section 4.14), the spread between the component fits should be reported before this function is treated as settled.

### Table A2. Pattern 1 and 2 parameters: a priori and calibrated values

| Parameter | Symbol | Units | Formal Model | Calibrated | Range explored |
|:---|:---:|:---|---:|---:|:---:|
| **Pattern 1 — in-nest development** | | | | | |
| `OSMIA_EGGDEVELTHRESHOLD` | *T*₀,egg | °C | 13.8 | 0.518 | 0 – 18 |
| `OSMIA_EGGDEVELDD` | *SET*egg | degree-days | 37.0 | 104.435 | 30 – 150 |
| `OSMIA_LARVADEVELTHRESHOLD` | *T*₀,larva | °C | 8.5 | 8.834 | 0 – 18 |
| `OSMIA_LARVADEVELDD` | *SET*larva | degree-days | 422.4 | 305.235 | 300 – 700 |
| `OSMIA_PREPUPADEVELDAYS` | *D*prepupa | days | 24.3 | 21.270 | 15 – 60 |
| `OSMIA_PREPUPALRATE_A/B/C/TOPT` | *r*(*T*) | — | Table A1b | as Formal Model | fixed, not fitted |
| — (prepupal variation) | — | proportion | ±10% | ±10% | fixed, not fitted |
| `OSMIA_PUPADEVELTHRESHOLD` | *T*₀,pupa | °C | 13.2 | 2.463 | 0 – 18 |
| `OSMIA_PUPADEVELDD` | *SET*pupa | degree-days | 272.3 | 555.907 | 200 – 700 |
| **Pattern 2 — overwintering and emergence** | | | | | |
| `OSMIA_INCOCOONOVERWINTERINGTEMPTHRESHOLD` | — | °C | 0.0 | 7.758 | 0 – 10 |
| `OSMIA_INCOCOONEMERGENCETEMPTHRESHOLD` | — | °C | 12.0 | 10.085 | 0 – 18 |
| `OSMIA_INCOCOONEMERGENCECOUNTERCONST` | — | — | 39.4819 | 13.3685 | 10 – 60 |
| `OSMIA_INCOCOONEMERGENCECOUNTERSLOPE` | — | — | −0.0147 | −0.01613 | −0.10 – 0 |
| | | | | | |
| *Aggregate fit* | nRMSE | — | 1.497 | **0.039** | — |
| *Survival, Poznań / Regensburg* | — | % | 0 / 0 | 100 / 100 | — |
| *Emergence onset (target DOY 104)* | — | DOY | 138 | **103** | — |
| *Deterministic emergence-counter base* | — | — | — | 11 | must be ≥ 1 (Section 4.9) |

Fitted with differential evolution, seed 42, **200 generations**, population multiplier 12, seeded at the Formal Model vector, with both the persistence and emergence-spread penalties active (Section 4.5). The prepupal rate function and the ±10% individual variation are not fitted in either column; only the prepupal scale is free.

The "range explored" column is a **search bound, not a confidence interval**; see Section 7.4. Threshold upper bounds were set to 18 °C so that the constrained refits of Section 4.8 remained feasible above a 12 °C floor.

Survival of 100 / 100 indicates that the parameterisation is not falsified by the persistence pattern. It is not a measure of fit quality (Section 4.1).

### Table A3. Implied stage durations at a constant 20 °C

A degree-day pair is hard to read directly, so the same parameterisations are shown as the stage durations they imply at a constant 20 °C — within the laboratory range of the source studies and above every threshold in the table.

| Parameterisation | Egg | Larva | Prepupa | Pupa | Total | Prepupa at 22 °C |
|:---|---:|---:|---:|---:|---:|---:|
| Formal Model | 6.0 | 36.7 | 26.3 | 40.0 | 109.0 | 24.3 |
| Calibrated | 5.4 | 27.3 | 23.0 | 31.7 | 87.4 | 21.3 |
| *Observed, 20 °C* | *3* | *36* | *28* | *32* | *99* | — |

Observed values are Giejdasz & Fliszkiewicz (2016), female, from Ziółkowska et al. (2023), Table 2. These are constant-temperature laboratory values and are **not** the fitting targets, which are the two field records; they are shown for orientation only.

The comparison is instructive in two ways. The Formal Model tracks the laboratory durations closely for the larva, prepupa and pupa — unsurprisingly, since it was fitted to them — yet fails under field forcing. And the calibrated set is faster than the laboratory series at 20 °C for every stage, by 21% overall, while matching the field records to within a few per cent (Section 4.10). This is the laboratory-versus-field tension of Section 4.14 stated in units a reader can interpret: a linear degree-day model fitted to constant-temperature laboratory data does not transfer to fluctuating field temperatures, and in this model the discrepancy is absorbed into the base temperature.

Note that the larval stage shows the largest divergence from the laboratory value (27.3 against 36) despite having the one base temperature that survives calibration unchanged (Section 4.7). The compensation there falls on the degree-day requirement, which drops from 422.4 to 305.2 — a reminder that the (*T*₀, SET) pair moves as a unit even when one of its coordinates does not.

### Table A4. Pattern 4 parameters: configured and calibrated values

Estimated by regression on 1,305 paired cocoon/adult mass records, with no simulation. `OSMIAS_PROVISIONINGTOCOCOON` is held fixed because only its product with the slope is identified from these data.

| Parameter | Symbol | Description | Units | Configured | Calibrated | Source |
|:---|:---:|:---|:---|---:|---:|:---|
| `OSMIAS_PROVISIONINGTOCOCOON` | *k* | Multiplier from cocoon mass to provision mass | — | 3.247 | *held fixed* | Seidelmann (2006), measured directly |
| `OSMIA_FEMALEMASSFROMPROVMASSSLOPE` | *β* | Slope of adult mass on provision mass | mg mg⁻¹ | 0.2500 | **0.2324** | This study, regression through origin |
| `OSMIA_FEMALEMASSFROMPROVMASSCONST` | *α* | Intercept of adult mass on provision mass | mg | 4.00 | **0.0** | This study; not distinguishable from zero (*p* = 0.34) |
| `OSMIA_MINFEMALEMASS` | — | Lower bound on adult female mass | mg | 25.0 | 25.0 (retained) | 1 of 597 observations below. **See Section 3.5** |
| `OSMIA_MAXFEMALEMASS` | — | Upper bound on adult female mass | mg | 200.0 | 200.0 (retained, non-binding) | Max observed 147.7 mg; bound carries no empirical content |

Composite relation as calibrated: adult mass = 0.7546 × cocoon mass, *R*² = 0.936, residual SD 4.54 mg (females, *n* = 601).

### Table A5. Empirical provision-mass distribution

Derived as cocoon mass × 3.247 from the paired field records. Supersedes the undocumented 480 mg/day "standard good forage" input used in earlier testing.

| Sex | *n* | Mean | SD | 5% | 25% | 50% | 75% | 95% | Range |
|:---|---:|---:|---:|---:|---:|---:|---:|---:|:---|
| Female | 601 | 397.8 | 75.4 | 275 | 355 | 402 | 451 | 508 | 73–619 mg |
| Male | 704 | 243.2 | 55.8 | 162 | 208 | 238 | 271 | 355 | 76–455 mg |

### Table A6. Pattern 3 parameters: configured and provisionally fitted values

Fitted jointly against the Ivanov (2006) distribution with a consistency constraint between the two egg-number routes. **Provisional**: conditional on the planned-versus-achieved question in Section 4.12.

| Parameter | Description | Units | Configured | Documented in code | Provisional fit |
|:---|:---|:---|---:|---:|---:|
| `OSMIA_EGGSPERNESTPROBARGS` | BETA shape parameters for the first-nest draw | — | 1.0 4.00 | 1.8 5 | **2.277 5.940** |
| `OSMIA_TOTALNESTSPOSSIBLE` | Nests a female may complete in her lifetime | nests | 5 | — | **3.50** (see Section 7.3) |
| `OSMIA_MINNOEGGSINNEST` | Floor on nest size | eggs | 3 | — | 3 (retained) |
| `OSMIA_MAXNOEGGSINNEST` | Scale term in the first-nest draw | eggs | 30 | — | 30 (retained) |
| — (undocumented shift) | +2 eggs applied to 45% of nests | eggs | +2 at *p* = 0.45 | — | absorbed into the fit |

Fit quality: pooled nest size mean 7.69 against an observed 7.75, SD 4.24 against 4.42, Wasserstein distance 0.329 (against 2.728 as configured). The two egg-number routes agree to 5.1%.

### Table A7. Parameters not covered by Tables A1–A6

The remaining `OSMIA_*` configuration parameters, all governing processes that are **not calibrated in this paper**. They are tabulated because a reader evaluating the results above needs to know what was held fixed, and because the provenance column implements the recommendation of Section 8.1: *measured*, *estimated from literature*, *derived*, and *assumed* are distinguished rather than collapsed into a single "source" field. Values are identical in `Osmia_FormalModel.cfg` and `Osmia_Calibrated.cfg` unless noted.

**Provenance codes.** M = measured and cited; L = estimated from literature; D = derived under stated assumptions; **A = assumed, no empirical source**; P = placeholder awaiting work.

#### Foraging and provisioning (Stage 3 — not calibrated)

| Parameter | Value | Units | Prov. | Note |
|:---|---:|:---|:---:|:---|
| `OSMIA_POLLENGIVEUPTHRESHOLD` | 0.75 | — | **A** | MIDox states these "do not map to any measurable biological parameters; hence … are fitted parameters" |
| `OSMIA_POLLENGIVEUPRETURN` | 0.75 | — | **A** | As above |
| `OSMIA_POLLENSCORETOMG` | 0.8 | mg per score | **A** | As above |
| `OSMIA_MAXPOLLEN` | 2.5 | mg h⁻¹ | **A** | Collection ceiling added during earlier informal testing |
| `OSMIA_NECTAR_PER_DAY` | 20 | — | **A** | |
| `OSMIA_FORAGESTEPS` | 20 | steps | **A** | Search frequency; see MIDox on the radial-spoke algorithm |
| `OSMIA_FORAGEMASKSTEPSZ` | 34 | m | **D** | Computed by the source as `TYPICALHOMINGDISTANCE / (FORAGESTEPS − 1)` = 660/19. Setting it explicitly breaks that link; recompute by hand if either input changes |
| `OSMIA_FORAGE_MASK_STEP` | 50 | m | **A** | |
| `OSMIA_MAX_HALF_WIDTH_FORAGE_MASK` | 600 | m | **A** | |
| `OSMIA_DETAILEDMASKSTEP` | 1 | m | **A** | |
| `OSMIADENSITYDENPENDENTPOLLENREMOVALCONST` | 0.5 | — | **A** | Global scaler representing competition from other pollinators |

#### Movement, nest finding and nest availability

| Parameter | Value | Units | Prov. | Note |
|:---|---:|:---|:---:|:---|
| `OSMIA_TYPICALHOMINGDISTANCE` | 660 | m | L | *r*₅₀ from translocation experiments |
| `OSMIA_MAXHOMINGDISTANCE` | 1430 | m | L | *r*₉₀ from translocation experiments |
| `OSMIA_FEMALEFINDNESTATTEMPTNO` | 20 | attempts | **A** | |
| `OSMIA_GENMOVPROBTYPE` / `ARGS` | BETA, 10 5 | — | **A** | Short-range movement distribution |
| `OSMIA_DISPMOVPROBTYPE` / `ARGS` | BETA, 10 5 | — | **A** | Dispersal distribution; identical to the short-range pair, which appears to be a default rather than a finding |
| `OSMIA_NESTBYLEDATAFILE` | `OsmiaNestsByHabitat.txt` | — | **A** | **124 habitat-specific density pairs. Attributed to Koh et al. (2016) but see Section 6.6: only the habitat ranking is literature-derived, the absolute magnitude is assumed, and realised values are one thousandth of those tabulated owing to an undocumented scaler** |

#### Flying conditions

| Parameter | Value | Units | Prov. | Note |
|:---|---:|:---|:---:|:---|
| `OSMIA_MIN_TEMP_FOR_FLYING` | 6 | °C | ? | **Discrepancy.** Both configuration files carry 6 °C. The implementation documentation states "*O. bicornis* can be active outside the nest if the temperature > 13 °C", citing Ziółkowska et al. (2023). The two do not agree, and 6 °C is well below any reported flight threshold for the species. To be resolved before Stage 3 |
| `OSMIA_MAX_WIND_SPEED_FOR_FLYING` | 8 | m s⁻¹ | L | Agrees with the documentation |
| `OSMIA_MAX_PRECIP_FOR_FLYING` | 0.1 | mm | L | Agrees with the documentation |

#### Nest construction and adult life

| Parameter | Value | Units | Prov. | Note |
|:---|---:|:---|:---:|:---|
| `OSMIA_MINCELLCONSTRUCTTIME` | 1 | d | L | |
| `OSMIA_MAXCELLCONSTRUCTTIME` | 4 | d | L | |
| `OSMIA_PRENESTINGDURATION` | 2 | d | **A** | |
| `OSMIA_LIFESPAN` | 60 | d | L | Adult female maximum |
| `OSMIA_FEMALEBACKMORT` | 0.02 | d⁻¹ | **A** | Daily background adult mortality |

#### Mass bookkeeping

| Parameter | Value | Units | Prov. | Note |
|:---|---:|:---|:---:|:---|
| `OSMIAS_COCOONTOPROVISIONING` | 0.30797659 | — | **D** | Reciprocal of `PROVISIONINGTOCOCOON` = 3.247. Stored separately; the two must be kept consistent by hand |
| `OSMIA_LIFETIMECOCOONMASSLOSS` | 30.0 | % | **A** | Flagged as uncertain in the source |
| `OSMIATOTALCOCOONMASSLOSS` | 15.0 | % | **A** | |
| `OSMIATOTALCOCOONMASSLOSSRANGE` | 5.0 | % | **A** | |
| `OSMIA_ADULTMASSCLASSSTEP` | 10.0 | mg | **A** | Output binning only |
| `OSMIA_MINMALEMASS` | 88 | mg | ? | **Implausibly narrow.** The male range 88–105 mg spans 17 mg, against an observed male provision range of 76–455 mg (Table A5). Female bounds span 175 mg. Not investigated here; flagged because it is inconsistent with the paired data |
| `OSMIA_MAXMALEMASS` | 105.0 | mg | ? | As above |
| `OSMIA_MALEMINTARGETPROVISIONMASS` | 10.0 | mg | **A** | |

#### Sex ratio and cocoon-mass relations (Stage 4, not fitted)

| Parameter | *n* | Value | Prov. | Note |
|:---|---:|:---|:---:|:---|
| `OSMIA_SEXRATIOVSMOTHERSMASSLINEAR` | 2 | 0.0055 −0.1025 | **D** | Constrained by `target_sex_ratio.csv`, which is itself derived from fitted equations rather than observed, and treated as a prior specification to reproduce rather than as data (Section 4.2.4) |
| `OSMIA_SEXRATIOVSMOTHERSAGELOGISTIC` | 4 | 14.9026 0.0914 0.6032 −0.3921 | **D** | As above |
| `OSMIA_FEMALECOCOONMASSVSMOTHERSMASSLINEAR` | 2 | 0.3 65.1 | **D** | The source carries a superseded alternative, 0.46 63.85. Which is intended should be confirmed |
| `OSMIA_FEMALECOCOONMASSVSMOTHERSAGELOGISTIC` | 4 | 18.0409 104.1982 133.7415 −0.1769 | **D** | |

#### Background and overwintering mortality

| Parameter | Value | Units | Prov. | Note |
|:---|---:|:---|:---:|:---|
| `OSMIA_EGGDAILYMORT` | 0.0014 | d⁻¹ | **A** | |
| `OSMIA_LARVADAILYMORT` | 0.0014 | d⁻¹ | **A** | |
| `OSMIA_PREPUPADAILYMORT` | 0.003 | d⁻¹ | **A** | |
| `OSMIA_PUPADAILYMORT` | 0.003 | d⁻¹ | **A** | |
| `OSMIA_INCOCOONPREWINTERINGTEMPTHRESHOLD` | 15.0 | °C | L | Degree-days above this determine overwintering mortality |
| `OSMIA_INCOCOONWINTERMORTCONST` | −4.63 | — | **P** | **Not fitted.** Fittable against Bosch & Kemp (2004) in the same framework at low cost; those data are *O. cornuta* (Section 4.15) |
| `OSMIA_INCOCOONWINTERMORTSLOPE` | 0.05 | — | **P** | As above |
| `OSMIA_OVERWINTER_DEGREE_DAYS_INITIAL_SIMU` | 320 | DD | **A** | Seeds the first simulated winter, before any cohort has accumulated its own |

#### Parasitism (mechanistic pathway disabled by default, therefore untested)

| Parameter | Value | Prov. | Note |
|:---|---:|:---:|:---|
| `OSMIA_USEMECHANISTICPARASITOIDS` | false | — | **The mechanistic pathway is off. Every parameter below it is therefore inert and unexercised by any run reported here** |
| `OSMIA_PARASITISMPROBTOTIMECELLOPEN` | 0.0075 | **A** | The active pathway: probability per day a cell stands open |
| `OSMIA_BOMBYLIDPROB` | 0.5 | **A** | |
| `OSMIA_PARAS_DAILYMORT` | 24 × 1.0 | **P** | Two values per month; uniform, i.e. a placeholder with no seasonal structure |
| `OSMIA_PARAS_DISPERSAL` | 0.001 0.0001 | **A** | |
| `OSMIA_PARAS_STARTHIGHLOW` | 2.0 1.0 2.0 1.0 | **A** | |
| `OSMIA_PERCAPITAPARASITATIONCHANCE` | 0.00001 0.00002 | **A** | |

#### Resource thresholds and simulation control

| Parameter | Value | Prov. | Note |
|:---|---:|:---:|:---|
| `OSMIA_POLLEN_THRESHOLDS` | 24 × 1.0 | **P** | Two per month; uniform placeholder, no seasonal structure |
| `OSMIA_NECTAR_THRESHOLDS` | 24 × 1.0 | **P** | As above |
| `OSMIA_STARTNOS` | 50000 | **A** | Initial population; a Stage 5 quantity |
| `OSMIA_EMERGENCEPROBTYPE` / `ARGS` | DISCRETE, `8 7 9 24 20 8 6 5 5 4 4` | M | Giejdasz & Wasilewski, April 1st cohort — a single overwintering treatment and already the widest of six available (Section 4.2.2). Held fixed |
| `OSMIA_STORE_POPULATION_DYNAMICS`, `..._FILE` | false, `OsmiaPopulationDynamics.txt` | — | Output control, no model effect |

**Pesticide parameters** (15, omitted individually) are all inert in the runs reported here: `OSMIA_PPP_PROB` = 0, `OSMIA_PPP_KILL_RATE` = 0, and the two threshold parameters set to 10,000. `OSMIA_FEMALE_BACKGROUND_PESTICIDE_RESPONSE` is the only response pathway enabled. None has been calibrated or exercised.

**Reading the provenance column.** Of the parameters in this table, the large majority are marked **A** — assumed, with no empirical source — and a further group **P**, placeholders awaiting work. This is not a criticism of the model: parameters governing processes that have not yet been calibrated must hold *some* value, and assumed values are the honest way to hold a place. It is, however, the reason Section 7.5 classes everything downstream of Stage 3 as speculative, and the reason the parameter table is presented with a provenance column at all. A reader who takes the two configuration files at face value would otherwise have no way to distinguish `OSMIA_TYPICALHOMINGDISTANCE`, which is measured, from `OSMIA_POLLENGIVEUPTHRESHOLD`, which is not.

---

## Data and code availability

`[TODO]` — replace the descriptions below with public locations (repository URL and archived DOI) before submission. All items currently exist in the project directory.

- Extracted target CSVs with `source_cell` provenance (`calibration_data/`, 2,498 records, regenerable via `extract_calibration_data.py`).
- The standalone phenology port `osmia_phenology.py` and its validation suite `test_osmia_phenology.py`.
- `sweep_maxiter200.py` — the constrained-refit analysis of Section 4.8, guarded and unguarded, floors 0–13.
- `calibrate_pattern4.py` — regression estimation of the mass mapping, with diagnostics.
- `calibrate_pattern3.py` — joint fit of the eggs-per-nest distribution and `OSMIA_TOTALNESTSPOSSIBLE`.
- `cull_offset_scan.py`, `cull_boundary_refit.py` — the cull-date robustness analysis of Section 4.8.
- Standardised weather inputs with a validated common format.
- Machine-readable parameter files for the a priori and calibrated sets (`Osmia_FormalModel.cfg`, `Osmia_Calibrated.cfg`), replacing values previously embedded in code comments, with provenance notes.

Confirm that the Zenodo code snapshot corresponds to the **amended** source against which these results were produced (Section 3.3).

Note on reproducibility: `scipy.optimize.differential_evolution` does not guarantee a stable random stream across versions. Exact reproduction of the fitted values requires scipy 1.17.1; on other versions the structure and aggregate error reproduce but individual parameters will differ in the last decimals.

## References

Bosch, J. & Kemp, W.P. (2004) Effect of pre-wintering and wintering temperature regimes on weight loss, survival, and emergence time in the mason bee *Osmia cornuta* (Hymenoptera: Megachilidae). *Apidologie* 35(5): 469–479. https://doi.org/10.1051/apido:2004035

Bosch, J., Sgolastra, F. & Kemp, W.P. (2008) Life cycle ecophysiology of *Osmia* mason bees used as crop pollinators. In: *Bee Pollination in Agricultural Ecosystems*. Oxford University Press. https://doi.org/10.1093/acprof:oso/9780195316957.003.0006 

Giejdasz, K. & Fliszkiewicz, M. (2016) Effect of temperature treatment during development of *Osmia rufa* L., on mortality, emergence and longevity of adults. *Journal of Apicultural Science* 60(2): 221–232. https://doi.org/10.1515/jas-2016-0029

Giejdasz, K. & Wilkaniec, Z. (2002) Individual development of the red mason bee *Osmia rufa L., Megachilidae* under natural and laboratory conditions

Grimm, V., Revilla, E., Berger, U., Jeltsch, F., Mooij, W.M., Railsback, S.F., Thulke, H.-H., Weiner, J., Wiegand, T. & DeAngelis, D.L. (2005) Pattern-oriented modeling of agent-based complex systems: lessons from ecology. *Science* 310(5750): 987–991. https://doi.org/10.1126/science.1116681

Ivanov, S.P. (2006) The nesting of *Osmia rufa* (L.) (Hymenoptera, Megachilidae) in the Crimea: structure and composition of nests. *Entomological Review* 86(5): 524–533. https://doi.org/10.1134/S0013873806050046

Kehrberger, S. & Holzschuh, A. (2019) Warmer temperatures advance flowering in a spring plant more strongly than emergence of two solitary spring bee species. *PLOS ONE* 14(6): e0218824. https://doi.org/10.1371/journal.pone.0218824

Koh, I., Lonsdorf, E.V., Williams, N.M., Brittain, C., Isaacs, R., Gibbs, J. & Ricketts, T.H. (2016) Modeling the status, trends, and impacts of wild bee abundance in the United States. *PNAS* 113(1): 140–145. https://doi.org/10.1073/pnas.1517685113

Radmacher, S. & Strohm, E. (2011) Effects of constant and fluctuating temperatures on the development of the solitary bee *Osmia bicornis* (Hymenoptera: Megachilidae). *Apidologie* 42(6): 711–720. https://doi.org/10.1007/s13592-011-0078-9

Raw, A. (1972) The biology of the solitary bee *Osmia rufa* (L.) (Megachilidae). *Transactions of the Royal Entomological Society of London*. https://doi.org/10.1111/j.1365-2311.1972.tb00364.x 


Ruddle, N. et al. (2018) Effects of exposure to winter oilseed rape grown from thiamethoxam-treated seed on the red mason bee Osmia bicornis. *Environ Toxicol Chem.* 2018 Apr;37(4):1071-1083. https://doi.org/10.1002/etc.4034

Schmolke, A., Galic, N. & Hinarejos, S. (2023) SolBeePop: a model of solitary bee populations in agricultural landscapes. *Journal of Applied Ecology* 60: 2573–2585. https://doi.org/10.1111/1365-2664.14541

Schmolke, A., Galic, N., Roeben, V., Preuss, T.G., Miles, M. & Hinarejos, S. (2024) SolBeePop_ecotox: a population model for pesticide risk assessments of solitary bees. *Environmental Toxicology and Chemistry* 43: 2645–2661. https://doi.org/10.1002/etc.5990

Seidelmann, K. (2006) Open-cell parasitism shapes maternal investment patterns in the Red Mason bee *Osmia rufa*. *Behavioral Ecology* 17(5): 839–848. https://doi.org/10.1093/beheco/arl017

Seidelmann, K., Ulbrich, K. & Mielenz, N. (2010) Conditional sex allocation in the Red Mason bee, *Osmia rufa*. *Behavioral Ecology and Sociobiology* 64(3): 337–347. https://doi.org/10.1007/s00265-009-0850-2

Sgolastra, F. et al. (2012) Duration of prepupal summer dormancy regulates synchronization of adult diapause with winter temperatures in bees of the genus *Osmia*. *Journal of Insect Physiology*. https://doi.org/10.1016/j.jinsphys.2012.04.008 

Tasei, J.-N. & Picart, M. (1973) Observations sur le développement d'*Osmia cornuta* Latr. et *Osmia rufa* L. (Hymenoptera Megachilidae). *Apidologie* 4(4): 295–315. https://doi.org/10.1051/apido:19730402 

Topping, C.J. (2022) The Animal, Landscape and Man Simulation System (ALMaSS): a history, design, and philosophy. *Research Ideas and Outcomes* 8: e89919.

Topping, C.J., Hansen, T.S., Jensen, T.S., Jepsen, J.U., Nikolajsen, F. & Odderskær, P. (2003) ALMaSS, an agent-based model for animals in temperate European landscapes. *Ecological Modelling* 167(1–2): 65–82. https://doi.org/10.1016/S0304-3800(03)00173-X

Topping, C.J., Kondrup Marcussen, L., Thomsen, P. & Chetcuti, J. (2022) The Formal Model article format: justifying modelling intent and a critical review of data foundations through publication. *Food and Ecological Systems Modelling Journal* 3: e91024. https://doi.org/10.3897/fmj.3.91024

Ziółkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the solitary bee *Osmia bicornis* L. agent-based model. *Food and Ecological Systems Modelling Journal* 4: e102102. https://doi.org/10.3897/fmj.4.102102
