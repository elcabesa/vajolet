use bullet_lib::{
    game::{
        inputs,
    },
    nn::optimiser,
    trainer::{
        save::SavedFormat,
        schedule::{TrainingSchedule, TrainingSteps, lr, wdl},
        settings::LocalSettings,
    },
    value::{ValueTrainerBuilder, loader},
};

use std::fs;
use std::str::FromStr;

const HIDDEN_SIZE: usize = 512;
const SCALE: i32 = 400;
const QA: i16 = 255;
const QB: i16 = 64;
const THREADS: usize = 10;
const INITIAL_LR: f32 = 0.001;
const FINAL_LR: f32 = INITIAL_LR * 0.01;
const SUPERBATCHES: usize = 900;
const CHECKPOINT: &str = "checkpoints";

macro_rules! net_id {
    () => {
        "vajolet1"
    };
}


const NET_ID: &str = net_id!();

fn main() {
    let mut trainer = ValueTrainerBuilder::default()
        .use_threads(THREADS)
        .dual_perspective()
        .optimiser(optimiser::AdamW)
        .inputs(inputs::Chess768)
        .save_format(&[
            SavedFormat::id("l0w").round().quantise::<i16>(QA),
            SavedFormat::id("l0b").round().quantise::<i16>(QA),
            SavedFormat::id("l1w").round().quantise::<i16>(QB),
            SavedFormat::id("l1b").round().quantise::<i16>(QA * QB),
        ])
        .loss_fn(|output, target| output.sigmoid().squared_error(target))
        .build(|builder, stm_inputs, ntm_inputs| {
            // weights
            let l0 = builder.new_affine("l0", 768, HIDDEN_SIZE);
            let l1 = builder.new_affine("l1", 2 * HIDDEN_SIZE, 1);

            // inference
            let stm_hidden = l0.forward(stm_inputs).screlu();
            let ntm_hidden = l0.forward(ntm_inputs).screlu();
            let hidden_layer = stm_hidden.concat(ntm_hidden);
            l1.forward(hidden_layer)
        });

    fs::create_dir_all(CHECKPOINT).unwrap();
    let paths = fs::read_dir(CHECKPOINT).unwrap();
    let mut max = 0;
    for path in paths {
        let p = path.unwrap().path();
        let number = p.file_stem().unwrap().to_str().unwrap().split("-").last().unwrap();
        let number = i32::from_str(number).unwrap();
        if number > max {
            max = number;
        }
        //println!("name: {}", max);
    }
    let start = max + 1;
    let checkpoint = format!("{}/vajolet1-{}/", CHECKPOINT, max);


    let schedule = TrainingSchedule {
        net_id: NET_ID.to_string(),
        eval_scale: SCALE as f32,
        steps: TrainingSteps {
            batch_size: 16_384,
            batches_per_superbatch: 6104,
            start_superbatch: start as usize,
            end_superbatch: SUPERBATCHES,
        },
        wdl_scheduler: wdl::ConstantWDL { value: 0.0 },
        lr_scheduler: lr::Warmup{ inner: lr::CosineDecayLR { initial_lr: INITIAL_LR, final_lr: FINAL_LR, final_superbatch: SUPERBATCHES }, warmup_batches: 20},
        save_rate: 10
    };

    let settings = LocalSettings { threads: 1, test_set: None, output_directory: CHECKPOINT, batch_queue_size: 512 };

    let data_loader = loader::DirectSequentialDataLoader::new(&["data/train.bullet"]);

    if max > 0 {
        println!("loading checkpoint {}",checkpoint);
        trainer.load_from_checkpoint(checkpoint.as_str());
    }

    trainer.run(&schedule, &settings, &data_loader);

}
